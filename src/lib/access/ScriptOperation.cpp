#include "access/ScriptOperation.h"

#include <storage/PointerCalculator.h>
#include <storage/MutableVerticalTable.h>
#include <storage/TableBuilder.h>
#include <storage/FixedLengthVector.h>
#include <storage/AbstractTable.h>
#include <storage/Table.h>

#include <helper/Settings.h>
#include <helper/types.h>

#include "log4cxx/logger.h"

#include <cstdio>
#include <functional>

namespace hyrise {
namespace access {

namespace {
log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
auto _ = QueryParser::registerPlanOperation<ScriptOperation>("ScriptOperation");
}

ScriptOperation::ScriptOperation() {}

#ifdef WITH_V8

v8::Local<v8::String> OneByteString(v8::Isolate* isolate,
                                    const char* data,
                                    v8::String::NewStringType type = v8::String::kNormalString,
                                    int length = -1) {
  return v8::String::NewFromOneByte(isolate, reinterpret_cast<const uint8_t*>(data), type, length);
}

template <typename T>
void wrapAttributeVector(v8::Isolate* isolate,
                         std::shared_ptr<const T>& table,
                         size_t internal,
                         v8::Local<v8::Object>& obj);

// This is the context data that we use in the isolate data per process.
// Basically it contains a map of all available tables to the plan operation
// with given keys. The key is the offset in this table. The first tables in
// this list are always the input tables of the plan operation
struct IsolateContextData {
  std::vector<storage::c_atable_ptr_t> tables;
};



template <typename T>
void releaseTableSharedPtr(v8::Isolate* isolate,
                           v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>> persistentObj,
                           void* pData) {
  auto pspNative = reinterpret_cast<std::shared_ptr<const T>*>(pData);
  delete pspNative;
}


/// This is a helper function that frees allocated objects that were
/// created using new.
template <typename T>
void deleteAllocated(v8::Isolate* isolate,
                     v8::Persistent<v8::Value, v8::CopyablePersistentTraits<v8::Value>> persitentObj,
                     void* data) {
  auto native = reinterpret_cast<T*>(data);
  delete native;
}

void LogMessage(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::String::Utf8Value str(args[0]);
  LOG4CXX_DEBUG(_logger, *str);
  args.GetReturnValue().Set(v8::Undefined(args.GetIsolate()));
}

// This function is used to read a JS file from disk into a std::string object.
//
// @param name The name / relative path of the file
// @param suffix The suffix of the file, that defaults to ".j"
std::string readScript(const std::string& name, const std::string& suffix = ".js") {
  FILE* file;
  file = fopen((Settings::getInstance()->getScriptPath() + "/" + name + suffix).c_str(), "rb");
  if (file == nullptr)
    throw std::runtime_error("Could not find file " + name);

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = static_cast<int>(fread(&chars[i], 1, size - i, file));
    i += read;
  }
  fclose(file);
  std::string result(chars, size);
  delete[] chars;
  return result;
}

// Implementation of the helper method that allows to include other JavaScript
// file read from local files and compile and run them into the VM
void Include(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  for (int i = 0; i < args.Length(); i++) {
    v8::String::Utf8Value str(args[i]);

    std::string js_file;
    try {
      js_file = readScript(*str);
    }
    catch (std::exception& e) {
      args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, e.what())));
      return;
    }

    if (js_file.length() > 0) {
      v8::Handle<v8::String> source = OneByteString(isolate, js_file.c_str());
      v8::Handle<v8::Script> script = v8::Script::Compile(source);
      args.GetReturnValue().Set(script->Run());
      return;
    }
  }
  args.GetReturnValue().Set(v8::Undefined(isolate));
}

// Return the value Id for the Value given to this class, based on the type of
// the argument call the right method
void TableGetValueIdForValue(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  auto val = args[1];

  auto tabId = args.Length() > 2 ? args[2]->Uint32Value() : 0u;


  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto tab = static_cast<storage::AbstractTable*>(ptr);

  if (val->IsNumber()) {

    args.GetReturnValue().Set(v8::Integer::New(
        isolate,
        tab->getValueIdForValue<hyrise_int_t>(args[0]->Uint32Value(), val->IntegerValue(), false, tabId).valueId));
  } else {  // val == string
    v8::String::Utf8Value u(val->ToString());
    args.GetReturnValue().Set(v8::Integer::New(
        isolate, tab->getValueIdForValue<hyrise_string_t>(args[0]->Uint32Value(), *u, false, tabId).valueId));
  }
}

// These are the wrapped functions of the abstract table
void TableGetSize(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  size_t value = static_cast<storage::AbstractTable*>(ptr)->size();
  args.GetReturnValue().Set(v8::Integer::New(isolate, value));
}


// Implementation that returns number of columns of the table
void TableGetColumnCount(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  size_t value = static_cast<storage::AbstractTable*>(ptr)->columnCount();
  args.GetReturnValue().Set(v8::Integer::New(isolate, value));
}

// Simple Converter form std::string to v8::String
struct StringToV8String {
  static v8::Handle<v8::String> New(v8::Isolate* isolate, std::string a) { return OneByteString(isolate, a.c_str()); }
};

// Templated method to allow easy wrapping of the getValue<type> method from
// the table
template <typename In, typename Out>
void TableGetValue(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto value = static_cast<storage::AbstractTable*>(ptr)->getValue<In>(args[0]->Uint32Value(), args[1]->Uint32Value());
  args.GetReturnValue().Set(Out::New(isolate, value));
}

// Helper function that checks if the object has a property set that is called
// _isModifiable. This property defines if the table is modifiable or not
bool IsModifiable(const v8::FunctionCallbackInfo<v8::Value>& args) {
  return args.This()->Get(OneByteString(args.GetIsolate(), "_isModifiable"))->ToBoolean()->Value();
}

// methods to allow easy wrapping of the setValue<type> method from
// the table
void TableSetValueInt(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if (!IsModifiable(args)) {
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "Table is not modifiable.")));
    return;
  }
  void* ptr = wrap->Value();
  static_cast<storage::AbstractTable*>(ptr)
      ->setValue<hyrise_int_t>(args[0]->Uint32Value(), args[1]->Uint32Value(), args[2]->Int32Value());
  args.GetReturnValue().Set(v8::Undefined(isolate));
}

void TableSetValueFloat(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if (!IsModifiable(args)) {
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "Table is not modifiable.")));
    return;
  }
  void* ptr = wrap->Value();
  static_cast<storage::AbstractTable*>(ptr)->setValue<hyrise_float_t>(
      args[0]->Uint32Value(), args[1]->Uint32Value(), v8::Local<v8::Number>::Cast(args[2])->Value());
  args.GetReturnValue().Set(v8::Undefined(isolate));
}

void TableSetValueString(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if (!IsModifiable(args)) {
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "Table is not modifiable.")));
    return;
  }
  void* ptr = wrap->Value();
  v8::String::Utf8Value u(args[2]->ToString());
  static_cast<storage::AbstractTable*>(ptr)
      ->setValue<hyrise_string_t>(args[0]->Uint32Value(), args[1]->Uint32Value(), hyrise_string_t(*u));
  args.GetReturnValue().Set(v8::Undefined(isolate));
}

// Implementation of the function that calls storage::AbstractTable->resize() This
// function is required for all cases where new rows are appended to the table,
// if resize is not called the table will not know how many rows it should
// allocate
void TableResize(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if (!IsModifiable(args)) {
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "Table is not modifiable.")));
    return;
  }
  void* ptr = wrap->Value();
  auto tab = static_cast<storage::AbstractTable*>(ptr);
  auto casted = dynamic_cast<storage::MutableVerticalTable*>(tab);
  if (casted) {
    casted->resize(args[0]->Uint32Value());
  } else {
    auto casted2 = dynamic_cast<storage::Table*>(tab);
    casted2->resize(args[0]->Uint32Value());
  }

  args.GetReturnValue().Set(v8::Undefined(isolate));
}

// Returns an object of the value id
void TableGetValueId(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto vid = static_cast<storage::AbstractTable*>(ptr)->getValueId(args[0]->Uint32Value(), args[1]->Uint32Value());

  v8::Handle<v8::Object> templ = v8::Object::New(isolate);
  templ->Set(OneByteString(isolate, "valueId"), v8::Integer::New(isolate, vid.valueId));
  templ->Set(OneByteString(isolate, "tableId"), v8::Integer::New(isolate, vid.table));

  args.GetReturnValue().Set(templ);
}

// Returns the value ID of the value id
void TableGetValueIdV(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto vid =
      static_cast<storage::AbstractTable*>(ptr)->getValueId(args[0]->Uint32Value(), args[1]->Uint32Value()).valueId;
  args.GetReturnValue().Set(v8::Number::New(isolate, vid));
}

// Returns the value ID of the value id
void TableGetValueIdVRange(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);


  size_t col = args[0]->ToInteger()->Value();
  size_t row = args[1]->ToInteger()->Value();
  size_t stop = args[2]->ToInteger()->Value();

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();

  auto tab = static_cast<storage::AbstractTable*>(ptr);
  auto data = v8::Object::New(isolate);

  auto vids = new std::vector<value_id_t>;
  for (size_t i = row, j = 0; i < stop; ++i, ++j) {
    auto vid = tab->getValueId(col, i).valueId;
    vids->push_back(vid);
  }

  // We wrap the data in a persistent object to be able to free the vids once we are done
  data->SetIndexedPropertiesToExternalArrayData(&(vids->at(0)), v8::kExternalUnsignedIntArray, (stop - row));
  args.GetReturnValue().Set(v8::Handle<v8::Value>());
}


static void AttributeVectorGetSize(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  auto ptr = static_cast<storage::AbstractAttributeVector*>(wrap->Value());
  auto casted = dynamic_cast<storage::FixedLengthVector<value_id_t>*>(ptr);
  size_t value = casted->size();
  // return v8::Integer::New(args.GetIsolate(), value);
  args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), value));
}

void AttributeVectorGet(const v8::FunctionCallbackInfo<v8::Value>& args) {
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  auto ptr = static_cast<storage::AbstractAttributeVector*>(wrap->Value());
  auto casted = dynamic_cast<storage::FixedLengthVector<value_id_t>*>(ptr);

  auto col = args[0]->ToInteger()->Value();
  auto row = args[1]->ToInteger()->Value();

  value_id_t value = casted->get(col, row);
  args.GetReturnValue().Set(v8::Integer::New(args.GetIsolate(), value));
}

void AttributeVectorGetRange(const v8::FunctionCallbackInfo<v8::Value>& args) {

  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);


  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  auto ptr = static_cast<storage::AbstractAttributeVector*>(wrap->Value());
  auto casted = dynamic_cast<storage::FixedLengthVector<value_id_t>*>(ptr);

  size_t col = args[0]->ToInteger()->Value();
  size_t row = args[1]->ToInteger()->Value();
  size_t stop = args[2]->ToInteger()->Value();

  auto data = v8::Object::New(isolate);

  auto vids = new std::vector<value_id_t>();
  for (size_t i = row; i < stop; ++i) {
    auto vid = casted->get(col, i);
    vids->push_back(vid);
  }

  // FIXME: we have to make sure that data has no longer lifespan than data
  data->SetIndexedPropertiesToExternalArrayData(vids->data(), v8::kExternalUnsignedIntArray, (stop - row));
  args.GetReturnValue().Set(data);
}



// Represents the Internal id of the table in the input list of the plan
// operation
void GetInternalId(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(v8::Local<v8::Integer>::Cast(info.Data()));
}


template <typename T>
void wrapAttributeVector(v8::Isolate* isolate, std::shared_ptr<T>& table, size_t internal, v8::Local<v8::Object>& obj) {

  v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
  templ->SetInternalFieldCount(1);

  templ->Set(OneByteString(isolate, "size"), v8::FunctionTemplate::New(isolate, AttributeVectorGetSize)->GetFunction());
  templ->Set(OneByteString(isolate, "get"), v8::FunctionTemplate::New(isolate, AttributeVectorGet)->GetFunction());
  templ->Set(OneByteString(isolate, "getRange"),
             v8::FunctionTemplate::New(isolate, AttributeVectorGetRange)->GetFunction());

  templ->SetAccessor(
      OneByteString(isolate, "_internalId"), GetInternalId, nullptr, v8::Integer::New(isolate, internal));

  obj = templ->NewInstance();
  obj->SetInternalField(0, v8::External::New(isolate, table.get()));
}


void TableGetAttributeVectors(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto tab = static_cast<storage::AbstractTable*>(ptr);

  auto attr_vectors = tab->getAttributeVectors(args[0]->ToInteger()->Value());
  auto result = v8::Array::New(isolate, attr_vectors.size());

  size_t i = 0;
  for (auto& av : attr_vectors) {
    v8::Local<v8::Object> obj;
    wrapAttributeVector<storage::AbstractAttributeVector>(isolate, av.attribute_vector, i, obj);
    obj->Set(OneByteString(isolate, "attribute_vector"), obj);
    obj->Set(OneByteString(isolate, "attribute_offset"), v8::Number::New(isolate, av.attribute_offset));
    result->Set(i++, obj);
  }
  args.GetReturnValue().Set(result);
}

// Basic function to wrap an abstract table and expose the main methods, the
// most important methods are to access the number of fields, size and the
// valueId and values at a given set of column row coordinates
template <typename T>
void wrapTable(v8::Isolate* isolate, const std::shared_ptr<T>& table, size_t internal, v8::Local<v8::Object>& obj) {

  v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
  templ->SetInternalFieldCount(1);

  // Set wrapped methods
  templ->Set(OneByteString(isolate, "size"), v8::FunctionTemplate::New(isolate, TableGetSize));
  templ->Set(OneByteString(isolate, "columnCount"), v8::FunctionTemplate::New(isolate, TableGetColumnCount));
  templ->Set(OneByteString(isolate, "valueId"), v8::FunctionTemplate::New(isolate, TableGetValueId));
  templ->Set(OneByteString(isolate, "valueIdV"), v8::FunctionTemplate::New(isolate, TableGetValueIdV));
  templ->Set(OneByteString(isolate, "valueIdVRange"), v8::FunctionTemplate::New(isolate, TableGetValueIdVRange));
  templ->Set(OneByteString(isolate, "getValueInt"),
             v8::FunctionTemplate::New(isolate, TableGetValue<hyrise_int_t, v8::Number>));
  templ->Set(OneByteString(isolate, "getValueFloat"),
             v8::FunctionTemplate::New(isolate, TableGetValue<hyrise_float_t, v8::Number>));
  templ->Set(OneByteString(isolate, "getValueString"),
             v8::FunctionTemplate::New(isolate, TableGetValue<hyrise_string_t, StringToV8String>));

  templ->Set(OneByteString(isolate, "setValueInt"), v8::FunctionTemplate::New(isolate, TableSetValueInt));
  templ->Set(OneByteString(isolate, "setValueFloat"), v8::FunctionTemplate::New(isolate, TableSetValueFloat));
  templ->Set(OneByteString(isolate, "setValueString"), v8::FunctionTemplate::New(isolate, TableSetValueString));
  templ->Set(OneByteString(isolate, "resize"), v8::FunctionTemplate::New(isolate, TableResize));

  templ->Set(OneByteString(isolate, "getAttributeVectors"),
             v8::FunctionTemplate::New(isolate, TableGetAttributeVectors));

  // Map ValueIds to Values
  templ->Set(OneByteString(isolate, "getValueIdForValue"), v8::FunctionTemplate::New(isolate, TableGetValueIdForValue));

  templ->SetAccessor(
      OneByteString(isolate, "_internalId"), GetInternalId, nullptr, v8::Integer::New(isolate, internal));

  obj = templ->NewInstance();
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, false));

  obj->SetInternalField(0, v8::External::New(isolate, const_cast<typename std::remove_const<T>::type*>(table.get())));
}

// Create a pointer calculator based on the input, the function has two
// arguments, first the table and second the position list
void createPointerCalculator(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));

  // The base table
  auto object = v8::Local<v8::Object>::Cast(args[0]);
  auto internal = object->Get(OneByteString(isolate, "_internalId"))->Uint32Value();

  // Get the JS Array and build a vector
  auto positions = v8::Local<v8::Array>::Cast(args[1]);
  auto size = positions->Length();

  auto pos = new pos_list_t;
  for (size_t i = 0; i < size; ++i) {
    pos->push_back(positions->Get(v8::Integer::New(isolate, i))->Uint32Value());
  }

  // Create a new table based from the position list and input table
  auto result = std::make_shared<storage::PointerCalculator>(isoContext->tables[internal], pos[0]);
  isoContext->tables.push_back(result);

  v8::Local<v8::Object> obj;
  wrapTable(isolate, result, isoContext->tables.size() - 1, obj);
  args.GetReturnValue().Set(obj);
}

// Create an empty modifiable based on the input, the function has two
// arguments, first the table and second the position list
void CopyStructureModifiable(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));

  // The base table
  auto object = v8::Local<v8::Object>::Cast(args[0]);
  auto internal = object->Get(OneByteString(isolate, "_internalId"))->Uint32Value();

  // Create a new table based from the position list and input table
  auto result = isoContext->tables[internal]->copy_structure_modifiable();
  isoContext->tables.push_back(result);

  v8::Local<v8::Object> obj;
  wrapTable(isolate, result, isoContext->tables.size() - 1, obj);
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, true));

  args.GetReturnValue().Set(obj);
}

// Build a new table using the table builder object
//
// The first arguments are a list of the field names with types and names, the
// second argument are the groups of attributes
void BuildTable(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));

  if (!args[0]->IsArray() && !args[1]->IsArray()) {
    args.GetReturnValue().Set(
        isolate->ThrowException(OneByteString(isolate, "Arguments must be two arrays with field decls and groups")));
    return;
  }

  auto fields = v8::Local<v8::Array>::Cast(args[0]);
  auto groups = v8::Local<v8::Array>::Cast(args[1]);

  storage::TableBuilder::param_list list;
  for (size_t i = 0; i < fields->Length(); ++i) {
    auto tmp = v8::Local<v8::Object>::Cast(fields->Get(i));
    list.append().set_type(*v8::String::Utf8Value(tmp->Get(OneByteString(isolate, "type")))).set_name(
        *v8::String::Utf8Value(tmp->Get(OneByteString(isolate, "name"))));
  }

  for (size_t i = 0; i < groups->Length(); ++i) {
    auto tmp = v8::Local<v8::Integer>::Cast(groups->Get(i));
    list.appendGroup(tmp->Value());
  }

  storage::atable_ptr_t result = storage::TableBuilder::build(list);
  isoContext->tables.push_back(result);


  v8::Local<v8::Object> obj;
  wrapTable(isolate, result, isoContext->tables.size() - 1, obj);
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, true));

  args.GetReturnValue().Set(obj);
}

// Create a new vertical table based on the number of arguments in the input
// list. We use the internal id of the object to lookup all tables in the
// global table list.
void BuildVerticalTable(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));
  std::vector<storage::atable_ptr_t> tabs;
  for (int i = 0; i < args.Length(); ++i) {
    // The base table
    auto object = v8::Local<v8::Object>::Cast(args[i]);
    auto internal = object->Get(OneByteString(isolate, "_internalId"))->Uint32Value();
    // FIXME
    tabs.push_back(std::const_pointer_cast<storage::AbstractTable>(isoContext->tables[internal]));
  }

  // Create a new table based from the position list and input table
  auto result = std::make_shared<storage::MutableVerticalTable>(tabs);
  isoContext->tables.push_back(result);

  v8::Local<v8::Object> obj;
  wrapTable(isolate, result, isoContext->tables.size() - 1, obj);
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, false));

  args.GetReturnValue().Set(obj);
}

// The goal of the result helpers is to provide the necessary functions to
// present a usable result. This includes basically the following
// possibilities:
//   * create a PC with a pos List
//   * create a new modifiable table based on the old table meta data
//   * create a new modifiable table using the table builder
//   * create a combination as a vertical table
void ScriptOperation::createResultHelpers(v8::Isolate* isolate) {
  auto global = isolate->GetCurrentContext()->Global();

  global->Set(OneByteString(isolate, "createPointerCalculator"),
              v8::FunctionTemplate::New(isolate, createPointerCalculator)->GetFunction());
  global->Set(OneByteString(isolate, "copyStructureModifiable"),
              v8::FunctionTemplate::New(isolate, CopyStructureModifiable)->GetFunction());
  global->Set(OneByteString(isolate, "buildTable"), v8::FunctionTemplate::New(isolate, BuildTable)->GetFunction());
  global->Set(OneByteString(isolate, "buildVerticalTable"),
              v8::FunctionTemplate::New(isolate, BuildVerticalTable)->GetFunction());
  global->Set(OneByteString(isolate, "include"), v8::FunctionTemplate::New(isolate, Include)->GetFunction());
  global->Set(OneByteString(isolate, "log"), v8::FunctionTemplate::New(isolate, LogMessage)->GetFunction());
}

// Helper method that wraps the input of the plan operation into table objects
// that provide the necessary fields in the JS world. The most important
// methods are wrapped and available for callers in JS
v8::Handle<v8::Array> ScriptOperation::prepareInputs(v8::Isolate* isolate) {

  v8::Handle<v8::Array> result = v8::Array::New(isolate, input.size());
  std::vector<v8::Local<v8::Object>> locals(input.size());
  // Fill out the values
  for (size_t i = 0; i < input.size(); ++i) {
    // FIXME evil wrapper handling for our const inputs
    wrapTable(isolate, input.getTable(i), i, locals[i]);
    result->Set(i, locals[i]);
  }

  // Return the value through Close.
  return result;
}

v8::Handle<v8::Object> ScriptOperation::prepareParameters(v8::Isolate* isolate) {

  v8::Handle<v8::Object> templ = v8::Object::New(isolate);
  for (auto& k : _parameters) {
    const auto& kcstr = k.first.data();
    const auto& vcstr = k.second.data();
    templ->Set(OneByteString(isolate, kcstr, v8::String::kNormalString, k.first.size()),
               OneByteString(isolate, vcstr, v8::String::kNormalString, k.second.size()));
  }

  return templ;
}


#endif

void ScriptOperation::executePlanOperation() {

#ifdef WITH_V8
  // is this necessary?
  // v8::V8::SetFlagsFromString("--gdbjit --prof", 15);
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  if (isolate == nullptr) {
    isolate = v8::Isolate::New();
    isolate->Enter();
  }


  // Set the data in the isolate context
  auto isoContext = new IsolateContextData();
  for (const auto& t : input.allOf<storage::AbstractTable>()) {
    isoContext->tables.push_back(t);
  }
  isolate->SetData(0, isoContext);


  // Create a stack-allocated handle scope.
  v8::HandleScope handle_scope(isolate);

  // Create a new context.
  v8::Local<v8::Context> context = v8::Context::New(isolate);

  // Enter the created context for compiling and
  // running the script.
  v8::Context::Scope context_scope(context);

  // Create a string containing the JavaScript source code.
  auto content = readScript(_scriptName);
  if (content.size() == 0) {
    throw std::runtime_error("Script is empty, cannot run empty script: " + _scriptName);
  }

  v8::Handle<v8::String> source = OneByteString(isolate, content.c_str(), v8::String::kNormalString, content.size());

  // Add Helper Functions
  createResultHelpers(isolate);


  // Compile the source code.
  {
    v8::TryCatch trycatch;
    v8::Handle<v8::Script> script = v8::Script::Compile(source);
    if (script.IsEmpty()) {
      throw std::runtime_error(*v8::String::Utf8Value(trycatch.Exception()));
    }
    auto check = script->Run();
    if (check.IsEmpty()) {
      throw std::runtime_error(*v8::String::Utf8Value(trycatch.Exception()));
    }

    // Once the source is compiled there must be a method available whis is
    // called hyrise_run_op
    v8::Local<v8::Function> fun =
        v8::Local<v8::Function>::Cast(context->Global()->Get(OneByteString(isolate, "hyrise_run_op")));
    if (fun->IsFunction()) {
      // Call the plan op with the inputs we converted
      v8::Handle<v8::Value> argv[] = {prepareInputs(isolate), prepareParameters(isolate)};
      auto result = v8::Local<v8::Object>::Cast(fun->Call(fun, 2, argv));

      if (result.IsEmpty()) {
        throw std::runtime_error(*v8::String::Utf8Value(trycatch.Exception()));
      }

      // Unwrap the data and extract the shared_ptr for the result
      size_t internal = result->Get(OneByteString(isolate, "_internalId"))->Uint32Value();
      addResult(isoContext->tables[internal]);
    }
  }

  // free the isolation context data we use
  delete isoContext;

// Dispose the persistent context.
#endif
}

std::shared_ptr<PlanOperation> ScriptOperation::parse(const Json::Value& data) {
  auto op = std::make_shared<ScriptOperation>();
  op->setScriptName(data["script"].asString());


  for (const auto& v : data.getMemberNames()) {
    op->_parameters[v] = data[v].asString();
  }

  return op;
}
}
}
