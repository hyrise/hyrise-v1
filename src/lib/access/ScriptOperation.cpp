#include "access/ScriptOperation.h"

#include <storage/PointerCalculator.h>
#include <storage/MutableVerticalTable.h>
#include <storage/TableBuilder.h>
#include <storage/FixedLengthVector.h>

#include <helper/Settings.h>
#include <helper/types.h>

#include "log4cxx/logger.h"

#ifdef WITH_V8
#include <v8.h>
#endif

#include <cstdio>
#include <functional>

namespace hyrise { namespace access { 

namespace {
  log4cxx::LoggerPtr _logger(log4cxx::Logger::getLogger("hyrise.access"));
  auto _ = QueryParser::registerPlanOperation<ScriptOperation>("ScriptOperation");
}

ScriptOperation::ScriptOperation() {

}

#ifdef WITH_V8

template<typename T>
v8::Local<v8::Object> wrapAttributeVector(std::shared_ptr<const T> table, size_t internal);

// This is the context data that we use in the isolate data per process.
// Basically it contains a map of all available tables to the plan operation
// with given keys. The key is the offset in this table. The first tables in
// this list are always the input tables of the plan operation
struct IsolateContextData {
  std::vector<storage::c_atable_ptr_t> tables;
};



template<typename T>
void releaseTableSharedPtr(v8::Isolate* isolate, v8::Persistent<v8::Value> persistentObj, void* pData) {
  auto pspNative = reinterpret_cast<std::shared_ptr<const T>*>(pData);
  delete pspNative;
  // Manually dispose of the Persistent handle
  persistentObj.Dispose(isolate);
  persistentObj.Clear();
}


/// This is a helper function that frees allocated objects that were
/// created using new. 
template<typename T>
void deleteAllocated(v8::Isolate* isolate, v8::Persistent<v8::Value> persitentObj, void* data) {
  auto native = reinterpret_cast<T*>(data);
  delete native;
  persitentObj.Dispose(isolate);
  persitentObj.Clear();
}


/// Helper function that wraps the current local handle in a
/// persistent handle to register destructors and embedd native data
v8::Persistent<v8::Object> makePersistent(v8::Isolate *isolate, v8::Handle<v8::Object> object, void *embedded, v8::NearDeathCallback callback) {
  v8::Persistent<v8::Object> persistentObj( v8::Persistent<v8::Object>::New(isolate, object));
  persistentObj.MakeWeak(isolate, embedded, callback);
  return persistentObj;
}


v8::Handle<v8::Value> LogMessage(const v8::Arguments& args) {
  v8::String::Utf8Value str(args[0]);
  LOG4CXX_DEBUG(_logger, *str);
  return v8::Undefined();
}

// This function is used to read a JS file from disk into a std::string object.
//
// @param name The name / relative path of the file
// @param suffix The suffix of the file, that defaults to ".j"
std::string readScript(const std::string &name, const std::string &suffix = ".js") {
  FILE* file = fopen((Settings::getInstance()->getScriptPath() + "/" + name + suffix).c_str(), "rb");
  if (file == nullptr) throw std::runtime_error("Could not find file " + name);

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
v8::Handle<v8::Value> Include(const v8::Arguments& args) {
    for (int i = 0; i < args.Length(); i++) {
        v8::String::Utf8Value str(args[i]);

        std::string js_file;
        try {
          js_file = readScript(*str);
        } catch (std::exception &e) {
          return v8::ThrowException(v8::String::New(e.what()));
        }

        if(js_file.length() > 0) {
            v8::Handle<v8::String> source = v8::String::New(js_file.c_str());
            v8::Handle<v8::Script> script = v8::Script::Compile(source);
            return script->Run();
        }
    }
    return v8::Undefined();
}

// Return the value Id for the Value given to this class, based on the type of
// the argument call the right method
v8::Handle<v8::Value> TableGetValueIdForValue(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto val = args[1];

  auto tabId = args.Length() > 2 ? args[2]->Uint32Value() : 0u;


  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto tab = static_cast<AbstractTable*>(ptr);

  if (val->IsNumber()) {

    auto num = val->ToNumber();

    return handle_scope.Close(
        v8::Integer::New(
          tab->getValueIdForValue<hyrise_int_t>(args[0]->Uint32Value(), val->IntegerValue(), false, tabId ).valueId
        )
      );
  } else { // val == string 
    v8::String::Utf8Value u(val->ToString());
    return handle_scope.Close(
        v8::Integer::New(
          tab->getValueIdForValue<hyrise_string_t>(args[0]->Uint32Value(), *u, false, tabId ).valueId
        )
      );
  }
}

// These are the wrapped functions of the abstract table
v8::Handle<v8::Value> TableGetSize(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  size_t value = static_cast<AbstractTable*>(ptr)->size();
  return handle_scope.Close(v8::Integer::New(value));
}


// Implementation that returns number of columns of the table
v8::Handle<v8::Value> TableGetColumnCount(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  size_t value = static_cast<AbstractTable*>(ptr)->columnCount();
  return handle_scope.Close(v8::Integer::New(value));
}

// Simple Converter form std::string to v8::String
struct StringToV8String {
  static v8::Handle<v8::String> New(std::string a) {
    return v8::String::New(a.c_str());
  }
};

// Templated method to allow easy wrapping of the getValue<type> method from
// the table
template<typename In, typename Out>
v8::Handle<v8::Value> TableGetValue(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto value = static_cast<AbstractTable*>(ptr)->getValue<In>(args[0]->Uint32Value(), args[1]->Uint32Value());
  return handle_scope.Close(Out::New(value));
}

// Helper function that checks if the object has a property set that is called
// _isModifiable. This property defines if the table is modifiable or not
bool IsModifiable(const v8::Local<v8::Object> &ext) {
  return ext->Get(v8::String::New("_isModifiable"))->ToBoolean()->Value();
}

// methods to allow easy wrapping of the setValue<type> method from
// the table
v8::Handle<v8::Value> TableSetValueInt(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if(!IsModifiable(args.This())) {
    return v8::ThrowException(v8::String::New("Table is not modifiable."));
  }
  void* ptr = wrap->Value();
  static_cast<AbstractTable*>(ptr)->setValue<hyrise_int_t>(args[0]->Uint32Value(), args[1]->Uint32Value(),args[2]->Int32Value());
  return handle_scope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TableSetValueFloat(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if(!IsModifiable(args.This())) {
    return v8::ThrowException(v8::String::New("Table is not modifiable."));
  }
  void* ptr = wrap->Value();
  static_cast<AbstractTable*>(ptr)->setValue<hyrise_float_t>(args[0]->Uint32Value(), args[1]->Uint32Value(),v8::Local<v8::Number>::Cast(args[2])->Value());
  return handle_scope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TableSetValueString(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if(!IsModifiable(args.This())) {
    return v8::ThrowException(v8::String::New("Table is not modifiable."));
  }
  void* ptr = wrap->Value();
  v8::String::Utf8Value u(args[2]->ToString());
  static_cast<AbstractTable*>(ptr)->setValue<hyrise_string_t>(args[0]->Uint32Value(), args[1]->Uint32Value(),hyrise_string_t(*u));
  return handle_scope.Close(v8::Undefined());
}

// Implementation of the function that calls AbstractTable->resize() This
// function is required for all cases where new rows are appended to the table,
// if resize is not called the table will not know how many rows it should
// allocate
v8::Handle<v8::Value> TableResize(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  if(!IsModifiable(args.This())) {
    return v8::ThrowException(v8::String::New("Table is not modifiable."));
  }
  void* ptr = wrap->Value();
  auto tab = static_cast<AbstractTable*>(ptr);
  auto casted = dynamic_cast<MutableVerticalTable*>(tab);
  if (casted) {
    casted->resize(args[0]->Uint32Value());
  } else {
    auto casted2 = dynamic_cast<Table<>*>(tab);
    casted2->resize(args[0]->Uint32Value());
  }
  
  return handle_scope.Close(v8::Undefined());
}

// Returns an object of the value id
v8::Handle<v8::Value> TableGetValueId(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto vid = static_cast<AbstractTable*>(ptr)->getValueId(args[0]->Uint32Value(), args[1]->Uint32Value());
  
  v8::Handle<v8::Object> templ = v8::Object::New();
  templ->Set(v8::String::New("valueId"), v8::Integer::New(vid.valueId));
  templ->Set(v8::String::New("tableId"), v8::Integer::New(vid.table));

  return templ;
}

// Returns the value ID of the value id
v8::Handle<v8::Value> TableGetValueIdV(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto vid = static_cast<AbstractTable*>(ptr)->getValueId(args[0]->Uint32Value(), args[1]->Uint32Value()).valueId;
  return v8::Number::New(vid);
}

// Returns the value ID of the value id
v8::Handle<v8::Value> TableGetValueIdVRange(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  
  size_t col = args[0]->ToInteger()->Value();
  size_t row = args[1]->ToInteger()->Value();
  size_t stop =  args[2]->ToInteger()->Value();

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();

  auto tab = static_cast<AbstractTable*>(ptr);
  auto data = v8::Object::New();

  auto vids = new std::vector<value_id_t>;
  for(size_t i=row, j=0; i < stop; ++i, ++j) {
    auto vid = tab->getValueId(col, i).valueId;
    vids->push_back(vid);
  }

  // We wrap the data in a persistent object to be able to free the vids once we are done
  data->SetIndexedPropertiesToExternalArrayData( &(vids->at(0)), v8::kExternalUnsignedIntArray, (stop-row));
  auto persistent = makePersistent(isolate, data, vids, deleteAllocated<std::vector<value_id_t>>);
  return persistent;
}


v8::Handle<v8::Value> AttributeVectorGetSize(const v8::Arguments& args) {
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  auto ptr = static_cast<AbstractAttributeVector*>(wrap->Value());
  auto casted = dynamic_cast<FixedLengthVector<value_id_t>*>(ptr);
  size_t value = casted->size();
  return v8::Integer::New(value);
}

v8::Handle<v8::Value> AttributeVectorGet(const v8::Arguments& args) {
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  auto ptr = static_cast<AbstractAttributeVector*>(wrap->Value());
  auto casted = dynamic_cast<FixedLengthVector<value_id_t>*>(ptr);

  auto col = args[0]->ToInteger()->Value();
  auto row = args[1]->ToInteger()->Value();

  value_id_t value = casted->get(col, row);
  return v8::Integer::New(value);
}

v8::Handle<v8::Value> AttributeVectorGetRange(const v8::Arguments& args) {

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);


  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  auto ptr = static_cast<AbstractAttributeVector*>(wrap->Value());
  auto casted = dynamic_cast<FixedLengthVector<value_id_t>*>(ptr);

  size_t col = args[0]->ToInteger()->Value();
  size_t row = args[1]->ToInteger()->Value();
  size_t stop = args[2]->ToInteger()->Value();

  auto data = v8::Object::New();

  auto vids = new std::vector<value_id_t>();
  for(size_t i=row; i < stop; ++i) {
    auto vid = casted->get(col, i);
    vids->push_back(vid);
  }

  // FIXME: we have to make sure that data has no longer lifespan than data
  data->SetIndexedPropertiesToExternalArrayData( vids->data(), v8::kExternalUnsignedIntArray, (stop-row));
  return handle_scope.Close(data);
}


    

// Represents the Internal id of the table in the input list of the plan
// operation
v8::Handle<v8::Value> GetInternalId(v8::Local<v8::String> property, const v8::AccessorInfo &info) {
  return v8::Local<v8::Integer>::Cast(info.Data());
}


template<typename T>
v8::Local<v8::Object> wrapAttributeVector(std::shared_ptr<T> table, size_t internal) {

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
  templ->SetInternalFieldCount(1);

  templ->Set(v8::String::New("size"), v8::FunctionTemplate::New(AttributeVectorGetSize));
  templ->Set(v8::String::New("get"), v8::FunctionTemplate::New(AttributeVectorGet));
  templ->Set(v8::String::New("getRange"), v8::FunctionTemplate::New(AttributeVectorGetRange));

  templ->SetAccessor(v8::String::New("_internalId"), GetInternalId, nullptr, v8::Integer::New(internal));

  auto obj = templ->NewInstance();
  obj->SetInternalField(0, v8::External::New(table.get()));
  return handle_scope.Close(obj);
}


v8::Handle<v8::Value> TableGetAttributeVectors(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto tab = static_cast<AbstractTable*>(ptr);

  auto attr_vectors = tab->getAttributeVectors(args[0]->ToInteger()->Value());
  auto result = v8::Array::New(attr_vectors.size());

  size_t i=0;
  for(auto &av : attr_vectors) {
    auto obj = v8::Object::New();
    obj->Set(v8::String::New("attribute_vector"), wrapAttributeVector<AbstractAttributeVector>(av.attribute_vector, i));
    obj->Set(v8::String::New("attribute_offset"), v8::Number::New(av.attribute_offset));
    result->Set(i++, obj);
  }
  return result;
}


// Basic function to wrap an abstract table and expose the main methods, the
// most important methods are to access the number of fields, size and the
// valueId and values at a given set of column row coordinates
template<typename T>
v8::Local<v8::Object> wrapTable(std::shared_ptr<const T> table, size_t internal) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Handle<v8::ObjectTemplate> templ = v8::ObjectTemplate::New();
  templ->SetInternalFieldCount(1);

  // Set wrapped methods
  templ->Set(v8::String::New("size"), v8::FunctionTemplate::New(TableGetSize));
  templ->Set(v8::String::New("columnCount"), v8::FunctionTemplate::New(TableGetColumnCount));
  templ->Set(v8::String::New("valueId"), v8::FunctionTemplate::New(TableGetValueId));
  templ->Set(v8::String::New("valueIdV"), v8::FunctionTemplate::New(TableGetValueIdV));
  templ->Set(v8::String::New("valueIdVRange"), v8::FunctionTemplate::New(TableGetValueIdVRange));
  templ->Set(v8::String::New("getValueInt"), v8::FunctionTemplate::New(TableGetValue<hyrise_int_t, v8::Number>));
  templ->Set(v8::String::New("getValueFloat"), v8::FunctionTemplate::New(TableGetValue<hyrise_float_t, v8::Number>));
  templ->Set(v8::String::New("getValueString"), v8::FunctionTemplate::New(TableGetValue<hyrise_string_t, StringToV8String>));
  
  templ->Set(v8::String::New("setValueInt"), v8::FunctionTemplate::New(TableSetValueInt));
  templ->Set(v8::String::New("setValueFloat"), v8::FunctionTemplate::New(TableSetValueFloat));
  templ->Set(v8::String::New("setValueString"), v8::FunctionTemplate::New(TableSetValueString));
  templ->Set(v8::String::New("resize"), v8::FunctionTemplate::New(TableResize));

  templ->Set(v8::String::New("getAttributeVectors"), v8::FunctionTemplate::New(TableGetAttributeVectors));

  // Map ValueIds to Values
  templ->Set(v8::String::New("getValueIdForValue"), v8::FunctionTemplate::New(TableGetValueIdForValue));

  templ->SetAccessor(v8::String::New("_internalId"), GetInternalId, nullptr, v8::Integer::New(internal));

  auto obj = templ->NewInstance();
  obj->Set(v8::String::New("_isModifiable"), v8::Boolean::New(false));
  obj->SetInternalField(0, v8::External::New(const_cast<T*>(table.get())));

  auto persistentObj = makePersistent(isolate, obj, new std::shared_ptr<const T>(table), releaseTableSharedPtr<T>);
  return handle_scope.Close(persistentObj);
}


// Create a pointer calculator based on the input, the function has two
// arguments, first the table and second the position list
v8::Handle<v8::Value> createPointerCalculator(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  IsolateContextData *isoContext = static_cast<IsolateContextData*>(isolate->GetData());

  // The base table
  auto object = v8::Local<v8::Object>::Cast(args[0]);
  auto internal = object->Get(v8::String::New("_internalId"))->Uint32Value();

  // Get the JS Array and build a vector
  auto positions = v8::Local<v8::Array>::Cast(args[1]);
  auto size = positions->Length();

  auto pos = new pos_list_t;
  for(size_t i=0; i < size; ++i) {
    pos->push_back(positions->Get(v8::Integer::New(i))->Uint32Value());
  }

  // Create a new table based from the position list and input table
  auto result = std::make_shared<PointerCalculator>(isoContext->tables[internal], pos);
  isoContext->tables.push_back(result);
  auto obj = wrapTable<PointerCalculator>(result, isoContext->tables.size()-1);
  return handle_scope.Close(obj);
}

// Create an empty modifiable based on the input, the function has two
// arguments, first the table and second the position list
v8::Handle<v8::Value> CopyStructureModifiable(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  IsolateContextData *isoContext = static_cast<IsolateContextData*>(isolate->GetData());

  // The base table
  auto object = v8::Local<v8::Object>::Cast(args[0]);
  auto internal = object->Get(v8::String::New("_internalId"))->Uint32Value();

  // Create a new table based from the position list and input table
  auto result = isoContext->tables[internal]->copy_structure_modifiable();
  isoContext->tables.push_back(result);
  auto obj = wrapTable<AbstractTable>(result, isoContext->tables.size()-1);
  obj->Set(v8::String::New("_isModifiable"), v8::Boolean::New(true));

  return handle_scope.Close(obj);
}

// Build a new table using the table builder object
//
// The first arguments are a list of the field names with types and names, the
// second argument are the groups of attributes
v8::Handle<v8::Value> BuildTable(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  IsolateContextData *isoContext = static_cast<IsolateContextData*>(isolate->GetData());

  if (!args[0]->IsArray() && !args[1]->IsArray()) {
    return v8::ThrowException(v8::String::New("Arguments must be two arrays with field decls and groups"));
  }

  auto fields = v8::Local<v8::Array>::Cast(args[0]);
  auto groups = v8::Local<v8::Array>::Cast(args[1]);

  TableBuilder::param_list list;
  for(size_t i=0; i < fields->Length(); ++i ) {
    auto tmp = v8::Local<v8::Object>::Cast(fields->Get(i));
    list.append().set_type(*v8::String::Utf8Value(tmp->Get(v8::String::New("type")))).set_name(
      *v8::String::Utf8Value(tmp->Get(v8::String::New("name"))));
  }

  for(size_t i=0; i < groups->Length(); ++i ) {
    auto tmp = v8::Local<v8::Integer>::Cast(groups->Get(i));
    list.appendGroup(tmp->Value());
  }

  
  storage::atable_ptr_t  result = TableBuilder::build(list);
  isoContext->tables.push_back(result);
  auto obj = wrapTable<AbstractTable>(result, isoContext->tables.size()-1);
  obj->Set(v8::String::New("_isModifiable"), v8::Boolean::New(true));

  return handle_scope.Close(obj);
}

// Create a new vertical table based on the number of arguments in the input
// list. We use the internal id of the object to lookup all tables in the 
// global table list.
v8::Handle<v8::Value> BuildVerticalTable(const v8::Arguments& args) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  IsolateContextData *isoContext = static_cast<IsolateContextData*>(isolate->GetData());
  std::vector<storage::atable_ptr_t> tabs;
  for(int i=0; i < args.Length(); ++i) {
    // The base table
    auto object = v8::Local<v8::Object>::Cast(args[i]);
    auto internal = object->Get(v8::String::New("_internalId"))->Uint32Value();
    // FIXME
    tabs.push_back(std::const_pointer_cast<AbstractTable>(isoContext->tables[internal]));
  }

  // Create a new table based from the position list and input table
  auto result = std::make_shared<MutableVerticalTable>(tabs);
  isoContext->tables.push_back(result);
  auto obj = wrapTable<AbstractTable>(result, isoContext->tables.size()-1);
  obj->Set(v8::String::New("_isModifiable"), v8::Boolean::New(false));

  return handle_scope.Close(obj);
}

// The goal of the result helpers is to provide the necessary functions to
// present a usable result. This includes basically the following
// possibilities:
//   * create a PC with a pos List
//   * create a new modifiable table based on the old table meta data
//   * create a new modifiable table using the table builder
//   * create a combination as a vertical table
void ScriptOperation::createResultHelpers(v8::Persistent<v8::Context> &context) {
  auto global = context->Global();

  global->Set(v8::String::New("createPointerCalculator"), v8::FunctionTemplate::New(createPointerCalculator)->GetFunction());
  global->Set(v8::String::New("copyStructureModifiable"), v8::FunctionTemplate::New(CopyStructureModifiable)->GetFunction());
  global->Set(v8::String::New("buildTable"), v8::FunctionTemplate::New(BuildTable)->GetFunction());
  global->Set(v8::String::New("buildVerticalTable"), v8::FunctionTemplate::New(BuildVerticalTable)->GetFunction());
  global->Set(v8::String::New("include"), v8::FunctionTemplate::New(Include)->GetFunction());
  global->Set(v8::String::New("log"), v8::FunctionTemplate::New(LogMessage)->GetFunction());
}

// Helper method that wraps the input of the plan operation into table objects
// that provide the necessary fields in the JS world. The most important
// methods are wrapped and available for callers in JS
v8::Handle<v8::Array> ScriptOperation::prepareInputs() {

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Handle<v8::Array> result = v8::Array::New(input.size());
  // Fill out the values
  for(size_t i=0; i < input.size(); ++i) {
    // FIXME evil wrapper handling for our const inputs
    result->Set(i, wrapTable<AbstractTable>(input.getTable(i), i));
  }
  
  // Return the value through Close.
  return handle_scope.Close(result);
}

v8::Handle<v8::Object> ScriptOperation::prepareParameters() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);  

  v8::Handle<v8::Object> templ = v8::Object::New();
  for(auto& k : _parameters) {
    const auto& kcstr = k.first.data();
    const auto& vcstr = k.second.data();
    templ->Set(v8::String::New(kcstr, k.first.size()), v8::String::New(vcstr, k.second.size()));
  }
  return handle_scope.Close(templ);
}


#endif

void ScriptOperation::executePlanOperation() {

#ifdef WITH_V8
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  if (isolate == nullptr)
  {
    isolate = v8::Isolate::New();
    isolate->Enter();
  }
  

  // Set the data in the isolate context
  auto isoContext = new IsolateContextData();
  for( const auto& t : input.allOf<AbstractTable>()) {
    isoContext->tables.push_back(t);
  }
  isolate->SetData(isoContext);


  // Create a stack-allocated handle scope.
  v8::HandleScope handle_scope(isolate);

  // Create a new context.
  v8::Persistent<v8::Context> context = v8::Context::New();
  
  // Enter the created context for compiling and
  // running the script. 
  v8::Context::Scope context_scope(context);

  // Create a string containing the JavaScript source code.
  auto content = readScript(_scriptName);
  if (content.size() == 0) {
    throw std::runtime_error("Script is empty, cannot run empty script: " + _scriptName);
  }

  v8::Handle<v8::String> source = v8::String::New(content.c_str(), content.size());

  // Add Helper Functions
  createResultHelpers(context);


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
    v8::Local<v8::Function> fun = v8::Local<v8::Function>::Cast(context->Global()->Get(v8::String::New("hyrise_run_op")));
    if (fun->IsFunction()) {
      // Call the plan op with the inputs we converted
      v8::Handle<v8::Value> argv[] = {prepareInputs(), prepareParameters()};
      auto result = v8::Local<v8::Object>::Cast(fun->Call(fun, 2, argv));
      if (result.IsEmpty()) {
        throw std::runtime_error(*v8::String::Utf8Value(trycatch.Exception()));
      }
      // Unwrap the data and extract the shared_ptr for the result
      size_t internal = result->Get(v8::String::New("_internalId"))->Uint32Value();
      addResult(isoContext->tables[internal]);
    }
  }

  // free the isolation context data we use
  delete isoContext;

  // Dispose the persistent context.
  context.Dispose(isolate);
#endif 

}

std::shared_ptr<PlanOperation> ScriptOperation::parse(const Json::Value &data) {
  auto op = std::make_shared<ScriptOperation>();
  op->setScriptName(data["script"].asString());


  for(const auto& v : data.getMemberNames()) {
    op->_parameters[v] = data[v].asString();
  }

  return op;
}

}}

