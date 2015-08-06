#include "access/ScriptOperation.h"

#include <storage/PointerCalculator.h>
#include <storage/MutableVerticalTable.h>
#include <storage/TableBuilder.h>
#include <storage/FixedLengthVector.h>
#include <storage/AbstractTable.h>
#include <storage/Table.h>

// MF execute query from js
#include <taskscheduler/AbstractTaskScheduler.h>
#include <taskscheduler/SharedScheduler.h>
#include <io/TXContext.h>
#include <helper/sha1.h>
#include <io/TransactionManager.h>
#include <access/system/QueryTransformationEngine.h>
#include <access/system/ResponseTask.h>
#include <boost/lexical_cast.hpp>
#include "helper/PapiTracer.h"

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

enum getValueType {
  getValueIntegerType,
  getValueFloatType,
  getValueStringType
};

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

v8::Handle<v8::String> toJsonString(v8::Isolate* isolate, v8::Handle<v8::Value> object) {
  v8::EscapableHandleScope scope(isolate);

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();

  v8::Handle<v8::Object> JSON = global->Get(OneByteString(isolate, "JSON"))->ToObject();
  v8::Handle<v8::Function> JSON_stringify =
      v8::Handle<v8::Function>::Cast(JSON->Get(OneByteString(isolate, "stringify")));

  return scope.Escape(v8::Local<v8::String>::Cast(JSON_stringify->Call(JSON, 1, &object)));
}

void LogMessage(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::String::Utf8Value str(toJsonString(args.GetIsolate(), args[0]));
  std::cout << *str << std::endl;
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

void TableMap(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  auto f = v8::Local<v8::Function>::Cast(args[0]);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto table = static_cast<storage::AbstractTable*>(ptr);

  std::vector<std::function<v8::Local<v8::Value>(const field_t, const size_t)>> getValueFunctions;
  auto getValueInt = [&](const field_t f, const size_t s) {
    hyrise_int_t val = table->getValue<hyrise_int_t>(f, s);
    return v8::Number::New(isolate, val);
  };
  auto getValueFloat = [&](const field_t f, const size_t s) {
    hyrise_float_t val = table->getValue<hyrise_float_t>(f, s);
    return v8::Number::New(isolate, val);
  };
  auto getValueString = [&](const field_t f, const size_t s) {
    hyrise_string_t val = table->getValue<hyrise_string_t>(f, s);
    return StringToV8String::New(isolate, val);
  };

  getValueFunctions.push_back(getValueInt);
  getValueFunctions.push_back(getValueFloat);
  getValueFunctions.push_back(getValueString);

  std::vector<getValueType> columnTypeDatatypeMap;
  columnTypeDatatypeMap.reserve(table->columnCount());

  for (size_t i = 0; i < table->columnCount(); ++i) {
    auto md = table->metadataAt(i);

    switch (md.getType()) {
      case IntegerType:
      case IntegerTypeDelta:
      case IntegerTypeDeltaConcurrent:
      case IntegerNoDictType:
        columnTypeDatatypeMap.push_back(getValueIntegerType);
        break;

      case FloatType:
      case FloatTypeDelta:
      case FloatTypeDeltaConcurrent:
        columnTypeDatatypeMap.push_back(getValueFloatType);
        break;

      case StringType:
      case StringTypeDelta:
      case StringTypeDeltaConcurrent:
        columnTypeDatatypeMap.push_back(getValueStringType);
        break;

      default:
        break;
    }
  }

  v8::Local<v8::Value>* cols = (v8::Local<v8::Value>*)malloc(sizeof(v8::Local<v8::Value>) * table->columnCount());

  for (size_t row = 0; row < table->size(); ++row) {
    for (size_t column = 0; column < table->columnCount(); ++column) {
      cols[column] = getValueFunctions[columnTypeDatatypeMap[column]](column, row);
    }
    f->Call(global, table->columnCount(), cols);
  }

  free(cols);
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

void AsArray(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope handle_scope(isolate);
  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  auto table = static_cast<storage::AbstractTable*>(ptr);

  auto resultArray = v8::Array::New(isolate, table->columnCount());

  for (size_t i = 0; i < table->columnCount(); ++i) {
    auto md = table->metadataAt(i);
    auto column = v8::Array::New(isolate, table->size());

    switch (md.getType()) {
      case IntegerType:
      case IntegerTypeDelta:
      case IntegerTypeDeltaConcurrent:
      case IntegerNoDictType:
        for (size_t j = 0; j < table->size(); ++j) {
          auto val = table->getValue<hyrise_int_t>(i, j);
          column->Set(j, v8::Number::New(isolate, val));
        }
        break;

      case FloatType:
      case FloatTypeDelta:
      case FloatTypeDeltaConcurrent:
        for (size_t j = 0; j < table->size(); ++j) {
          auto val = table->getValue<hyrise_float_t>(i, j);
          column->Set(j, v8::Number::New(isolate, val));
        }
        break;

      case StringType:
      case StringTypeDelta:
      case StringTypeDeltaConcurrent:
        for (size_t j = 0; j < table->size(); ++j) {
          auto val = table->getValue<std::string>(i, j);
          column->Set(j, StringToV8String::New(isolate, val));
        }
        break;

      default:
        break;
    }

    resultArray->Set(i, column);
  }

  args.GetReturnValue().Set(resultArray);
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

  templ->Set(OneByteString(isolate, "size"), v8::FunctionTemplate::New(isolate, AttributeVectorGetSize));
  templ->Set(OneByteString(isolate, "get"), v8::FunctionTemplate::New(isolate, AttributeVectorGet));
  templ->Set(OneByteString(isolate, "getRange"), v8::FunctionTemplate::New(isolate, AttributeVectorGetRange));

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
void wrapTable(v8::Isolate* isolate, std::shared_ptr<const T> table, size_t internal, v8::Local<v8::Object>& obj) {

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

  templ->Set(OneByteString(isolate, "asArray"), v8::FunctionTemplate::New(isolate, AsArray));

  // Map ValueIds to Values
  templ->Set(OneByteString(isolate, "getValueIdForValue"), v8::FunctionTemplate::New(isolate, TableGetValueIdForValue));

  templ->Set(OneByteString(isolate, "map"), v8::FunctionTemplate::New(isolate, TableMap));
  templ->Set(OneByteString(isolate, "filter"), v8::FunctionTemplate::New(isolate, TableFilter));

  templ->SetAccessor(
      OneByteString(isolate, "_internalId"), GetInternalId, nullptr, v8::Integer::New(isolate, internal));

  obj = templ->NewInstance();
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, false));
  obj->SetInternalField(0, v8::External::New(isolate, const_cast<T*>(table.get())));
}

void TableFilter(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));
  PapiTracer pt;
  epoch_t startTime = 0;

  if (isoContext->recordPerformance) {
    startTime = get_epoch_nanoseconds();

    pt.addEvent("PAPI_TOT_CYC");
    pt.start();
  }

  v8::Handle<v8::Object> global = isolate->GetCurrentContext()->Global();
  auto f = v8::Local<v8::Function>::Cast(args[0]);

  auto wrap = v8::Local<v8::External>::Cast(args.This()->GetInternalField(0));
  void* ptr = wrap->Value();
  // second parameter is null-deleter to prevent ptr from being double-freed
  auto table = storage::c_atable_ptr_t((const storage::AbstractTable*)ptr, [](const storage::AbstractTable*) {});

  std::vector<std::function<v8::Local<v8::Value>(const field_t, const size_t)>> getValueFunctions;
  auto getValueInt = [&](const field_t f, const size_t s) {
    hyrise_int_t val = table->getValue<hyrise_int_t>(f, s);
    return v8::Number::New(isolate, val);
  };
  auto getValueFloat = [&](const field_t f, const size_t s) {
    hyrise_float_t val = table->getValue<hyrise_float_t>(f, s);
    return v8::Number::New(isolate, val);
  };
  auto getValueString = [&](const field_t f, const size_t s) {
    hyrise_string_t val = table->getValue<hyrise_string_t>(f, s);
    return StringToV8String::New(isolate, val);
  };
  getValueFunctions.push_back(getValueInt);
  getValueFunctions.push_back(getValueFloat);
  getValueFunctions.push_back(getValueString);

  std::vector<getValueType> columnTypeDatatypeMap;
  columnTypeDatatypeMap.reserve(table->columnCount());

  for (size_t i = 0; i < table->columnCount(); ++i) {
    auto md = table->metadataAt(i);

    switch (md.getType()) {
      case IntegerType:
      case IntegerTypeDelta:
      case IntegerTypeDeltaConcurrent:
      case IntegerNoDictType:
        columnTypeDatatypeMap.push_back(getValueIntegerType);
        break;

      case FloatType:
      case FloatTypeDelta:
      case FloatTypeDeltaConcurrent:
        columnTypeDatatypeMap.push_back(getValueFloatType);
        break;

      case StringType:
      case StringTypeDelta:
      case StringTypeDeltaConcurrent:
        columnTypeDatatypeMap.push_back(getValueStringType);
        break;

      default:
        break;
    }
  }

  v8::Local<v8::Value>* cols = (v8::Local<v8::Value>*)malloc(sizeof(v8::Local<v8::Value>) * table->columnCount());
  auto positionList = pos_list_t();

  for (size_t row = 0; row < table->size(); ++row) {
    for (size_t column = 0; column < table->columnCount(); ++column) {
      try {
        cols[column] = getValueFunctions[columnTypeDatatypeMap[column]](column, row);
      }
      catch (std::exception& e) {
        args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, e.what())));
        return;
      }
    }
    auto result = v8::Local<v8::Value>::Cast(f->Call(global, table->columnCount(), cols));
    if (result->BooleanValue())
      positionList.push_back(row);
  }

  free(cols);

  auto outputTable = std::make_shared<storage::PointerCalculator>(table, positionList);
  isoContext->tables.push_back(outputTable);

  convertDataflowDataToJson(isolate, isoContext->tables.size() - 1, outputTable->size(), isoContext->jsonQueryDataflow);

  v8::Local<v8::Object> obj;
  wrapTable<storage::PointerCalculator>(isolate, outputTable, isoContext->tables.size() - 1, obj);

  if (isoContext->recordPerformance) {
    pt.stop();
    epoch_t endTime = get_epoch_nanoseconds();
    std::string threadId = boost::lexical_cast<std::string>(std::this_thread::get_id());
    performance_attributes_t* pa = new performance_attributes_t();
    *pa = (performance_attributes_t) {pt.value("PAPI_TOT_CYC"), 0,              "NO_PAPI",
                                      "filter",                 "noOperatorId", startTime,
                                      endTime,                  threadId,       std::numeric_limits<size_t>::max()};
    std::vector<std::unique_ptr<performance_attributes_t>> performanceVector;
    performanceVector.push_back(std::unique_ptr<performance_attributes_t>(pa));

    convertPerformanceDataToJson(isolate, performanceVector, 0, isoContext->jsonQueryPerformanceValues);
  }

  args.GetReturnValue().Set(obj);
}

// Create a pointer calculator based on the input, the function has two
// arguments, first the table and second the position list
void createPointerCalculator(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));

  // The base table
  auto object = v8::Local<v8::Object>::Cast(args[0]);
  if (!object->IsObject()) {
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "Argument invalid")));
    return;
  }

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
  wrapTable<storage::PointerCalculator>(isolate, result, isoContext->tables.size() - 1, obj);
  args.GetReturnValue().Set(obj);
}

// Create an empty modifiable based on the input, the function has two
// arguments, first the table and second the position list
void CopyStructureModifiable(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));

  // The base table
  auto object = v8::Local<v8::Object>::Cast(args[0]);
  if (!object->IsObject()) {
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "Argument invalid")));
    return;
  }

  auto internal = object->Get(OneByteString(isolate, "_internalId"))->Uint32Value();

  // Create a new table based from the position list and input table
  auto result = isoContext->tables[internal]->copy_structure_modifiable();
  isoContext->tables.push_back(result);

  v8::Local<v8::Object> obj;

  wrapTable<storage::AbstractTable>(isolate, result, isoContext->tables.size() - 1, obj);
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, true));

  args.GetReturnValue().Set(obj);
}

void BuildTableShort(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));

  if (!args[0]->IsObject()) {
    args.GetReturnValue().Set(
        isolate->ThrowException(OneByteString(isolate, "First argument must be a dictionary like: name -> type ...")));
    return;
  }

  auto fields = v8::Local<v8::Object>::Cast(args[0]);
  auto fieldNames = v8::Local<v8::Array>::Cast(fields->GetPropertyNames());

  storage::TableBuilder::param_list list;
  for (size_t i = 0; i < fieldNames->Length(); ++i) {
    v8::String::Utf8Value fieldName(fieldNames->Get(i));
    v8::String::Utf8Value fieldType(fields->Get(OneByteString(isolate, *(fieldName))));

    list.append().set_type(*(fieldType)).set_name(*(fieldName));
  }

  storage::atable_ptr_t result = storage::TableBuilder::build(list);
  isoContext->tables.push_back(result);

  if (args.Length() > 1 && args[1]->IsUint32()) {
    result->resize(args[1]->Uint32Value());
  }

  v8::Local<v8::Object> obj;
  wrapTable<storage::AbstractTable>(isolate, result, isoContext->tables.size() - 1, obj);
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, true));

  args.GetReturnValue().Set(obj);
}

void BuildTableColumn(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));

  if (!args[0]->IsObject() || !args[1]->IsUint32()) {
    args.GetReturnValue().Set(isolate->ThrowException(
        OneByteString(isolate, "First argument must be a dictionary like: name -> type. Second the table size.")));
    return;
  }

  auto fields = v8::Local<v8::Object>::Cast(args[0]);
  auto fieldNames = v8::Local<v8::Array>::Cast(fields->GetPropertyNames());

  std::vector<storage::atable_ptr_t> tabs;

  for (size_t i = 0; i < fieldNames->Length(); ++i) {
    storage::TableBuilder::param_list list;

    v8::String::Utf8Value fieldName(fieldNames->Get(i));
    v8::String::Utf8Value fieldType(fields->Get(OneByteString(isolate, *(fieldName))));

    list.append().set_type(*(fieldType)).set_name(*(fieldName));

    storage::atable_ptr_t tab = storage::TableBuilder::build(list);
    tab->resize(args[1]->Uint32Value());

    tabs.push_back(tab);
  }

  auto result = std::make_shared<storage::MutableVerticalTable>(tabs);
  isoContext->tables.push_back(result);

  result->resize(args[1]->Uint32Value());

  v8::Local<v8::Object> obj;
  wrapTable<storage::AbstractTable>(isolate, result, isoContext->tables.size() - 1, obj);

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

  if (!args[0]->IsArray() || !args[1]->IsArray()) {
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

  wrapTable<storage::AbstractTable>(isolate, result, isoContext->tables.size() - 1, obj);

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

  wrapTable<storage::AbstractTable>(isolate, result, isoContext->tables.size() - 1, obj);
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, false));

  args.GetReturnValue().Set(obj);
}

void printRecursive(const v8::CpuProfileNode* node, std::string prefix, std::stringstream& output) {
  output << prefix << *v8::String::Utf8Value(node->GetFunctionName()) << ":" << node->GetLineNumber() << ","
         << node->GetColumnNumber() << " - " << node->GetHitCount() << std::endl;

  for (int i = 0; i < node->GetChildrenCount(); i++)
    printRecursive(node->GetChild(i), prefix + "-", output);
}



std::string parseCpuProfile(v8::CpuProfile* cpuProfile) {
  std::stringstream s;

  s << "StartTime: " << cpuProfile->GetStartTime() << std::endl << "EndTime: " << cpuProfile->GetEndTime() << std::endl
    << "SampleCount: " << cpuProfile->GetSamplesCount() << std::endl;

  const v8::CpuProfileNode* node = cpuProfile->GetTopDownRoot();

  printRecursive(node, "|", s);

  return s.str();
}

void convertPerformanceDataToJson(v8::Isolate* isolate,
                                  performance_vector_t& perfDataVector,
                                  epoch_t queryStart,
                                  Json::Value& result) {
  Json::Value jsonPerfArray(Json::arrayValue);

  for (const auto& attr : perfDataVector) {
    Json::Value element;
    element["papi_event"] = Json::Value(attr->papiEvent);
    element["duration"] = Json::Value((Json::UInt64)attr->duration);
    element["data"] = Json::Value((Json::UInt64)attr->data);
    element["name"] = Json::Value(attr->name);
    element["id"] = Json::Value(attr->operatorId);
    element["startTime"] = Json::Value((double)(attr->startTime - queryStart) / 1000000);
    element["endTime"] = Json::Value((double)(attr->endTime - queryStart) / 1000000);
    element["executingThread"] = Json::Value(attr->executingThread);
    if (attr->cardinality != std::numeric_limits<size_t>::max())
      element["cardinality"] = Json::Value(attr->cardinality);

    jsonPerfArray.append(element);
  }

  // @todo what about deeper stacktraces?
  v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(isolate, 1);
  int lineNumber = -1;
  if (stackTrace->GetFrameCount()) {
    lineNumber = stackTrace->GetFrame(0)->GetLineNumber();
  }

  result[std::to_string(lineNumber)] = jsonPerfArray;
}

void convertDataflowDataToJson(v8::Isolate* isolate, size_t internalId, size_t cardinality, Json::Value& result) {
  // @todo what about deeper stacktraces?
  v8::Local<v8::StackTrace> stackTrace = v8::StackTrace::CurrentStackTrace(isolate, 1);
  int lineNumber = -1;
  if (stackTrace->GetFrameCount()) {
    lineNumber = stackTrace->GetFrame(0)->GetLineNumber();
  }

  if (result.isMember(std::to_string(internalId))) {
    result[std::to_string(internalId)][std::to_string(lineNumber)] = cardinality;
  } else {
    Json::Value idResult;
    idResult[std::to_string(lineNumber)] = cardinality;
    result[std::to_string(internalId)] = idResult;
  }
}

/**
 *  parses a json query string, generates tasks from it and executes them via the responseTask
 *
 *  basically copied from: void RequestParseTask::operator()() in RequestParseTask.cpp:67
 *
 */
void internalExecuteQuery(const std::string& query_string,
                          std::shared_ptr<ResponseTask> responseTask,
                          const storage::c_atable_ptr_t inputTable,
                          bool recordPerformance,
                          const std::string& papiEvent) {
  std::shared_ptr<hyrise::taskscheduler::AbstractTaskScheduler> scheduler;

  if (!taskscheduler::SharedScheduler::getInstance().isInitialized()) {
    //@TODO quick fix for no scheduler while unit testing, fix me, or maybe just dont unit test me :)
    taskscheduler::SharedScheduler::getInstance().init("CentralScheduler");
  }

  scheduler = taskscheduler::SharedScheduler::getInstance().getScheduler();
  epoch_t queryStart = get_epoch_nanoseconds();
  responseTask->setQueryStart(queryStart);

  std::vector<std::shared_ptr<hyrise::taskscheduler::Task>> tasks;

  Json::Value request_data;
  Json::Reader reader;

  if (reader.parse(query_string, request_data)) {
    responseTask->setRecordPerformanceData(recordPerformance);

    std::shared_ptr<hyrise::taskscheduler::Task> result = nullptr;

    try {
      tasks = QueryParser::instance().deserialize(QueryTransformationEngine::getInstance()->transform(request_data),
                                                  &result);
    }
    catch (const std::exception& ex) {
      // clean up, so we don't end up with a whole mess due to thrown exceptions
      responseTask->addErrorMessage(std::string("RequestParseTask: ") + ex.what());
      tasks.clear();
      result = nullptr;
    }

    if (result != nullptr) {
      responseTask->addDependency(result);
    } else {
      responseTask->addErrorMessage(std::string("Json did not yield tasks"));
    }

    for (const auto& func : tasks) {
      if (auto task = std::dynamic_pointer_cast<PlanOperation>(func)) {
        if (!papiEvent.empty()) {
          task->setEvent(papiEvent);
        }
        responseTask->registerPlanOperation(task);
        if (!task->hasSuccessors()) {
          responseTask->addDependency(task);
        }
      }
    }

    if (!tasks.empty()) {
      auto task = std::dynamic_pointer_cast<PlanOperation>(tasks[0]);
      if (task)
        task->addInput(inputTable);
    }

  } else {
    // Forward parsing error
    responseTask->addErrorMessage("Parsing: " + reader.getFormatedErrorMessages());
  }

  int number_of_tasks = tasks.size();
  std::vector<bool> isExecuted(number_of_tasks, false);
  int executedTasks = 0;
  while (executedTasks < number_of_tasks) {
    for (int i = 0; i < number_of_tasks; i++) {
      if (!isExecuted[i] && tasks[i]->isReady()) {
        (*tasks[i])();
        tasks[i]->notifyDoneObservers();
        executedTasks++;
        isExecuted[i] = true;
      }
    }
  }
  responseTask->setQueryStart(queryStart);
}

std::string getStringForPreparedStatement(v8::Isolate* isolate, v8::Local<v8::Object> object, std::string& key) {
  std::string returnString;

  auto value = object->Get(OneByteString(isolate, key.c_str()));
  if (value->IsUint32() || value->IsInt32()) {
    returnString = std::to_string(value->IntegerValue());
  } else if (value->IsNumber()) {
    returnString = std::to_string(value->NumberValue());
  } else if (value->IsBoolean()) {
    returnString = std::to_string(value->BooleanValue());
  } else if (value->IsString()) {
    v8::String::Utf8Value utf8String(value);

    returnString = *utf8String;
    returnString = "\"" + returnString + "\"";
  } else {
    throw std::runtime_error("Could not parse key: " + key + " - maybe it is not part of the dictionary?");
  }

  return returnString;
}

/**
 * Method exposed to JS for execution of JSON queries
 * JS-Parameters are <JSON-Query (string)> <input table>
 */
void ExecuteQuery(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  IsolateContextData* isoContext = static_cast<IsolateContextData*>(isolate->GetData(0));
  storage::c_atable_ptr_t inputTable;

  // check arguments

  if (args.Length() < 1) {
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "need at least json query")));
    return;
  }

  if ((args.Length() > 1) &&
      (!args[1]->IsNull())) {  // if second param (input table) is null, dont sanity check but ignore it
    auto object = v8::Local<v8::Object>::Cast(args[1]);
    if (!object->IsObject()) {
      args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, "input table (2. argument) invalid")));
      return;
    }
    auto internal = object->Get(OneByteString(isolate, "_internalId"))->Uint32Value();

    if (internal >= isoContext->tables.size()) {
      args.GetReturnValue().Set(
          isolate->ThrowException(OneByteString(isolate, "input table (2. argument) invalid/ idx outofrange")));
      return;
    }

    inputTable = isoContext->tables[internal];
  }

  v8::String::Utf8Value inputString(toJsonString(isolate, args[0]));
  std::string queryString = *inputString;

  if (args.Length() > 2) {
    // parameters for "preparedStatements" provided

    auto object = v8::Local<v8::Object>::Cast(args[2]);

    size_t position = queryString.find("#{");
    while (position != std::string::npos) {
      size_t positionEnd = queryString.find("}", position + 1);
      auto key = queryString.substr(position + 2, positionEnd - position - 2);

      std::string preparedString;
      try {
        preparedString = getStringForPreparedStatement(isolate, object, key);
      }
      catch (std::exception const& e) {
        args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, e.what())));
        return;
      }

      queryString.replace(position - 1, (positionEnd - position) + 3, preparedString);
      position = queryString.find("#{", position + 1);
    }
  }

  auto responseTask = std::make_shared<ResponseTask>(nullptr);

  // execute query

  internalExecuteQuery(queryString, responseTask, inputTable, isoContext->recordPerformance, isoContext->papiEvent);

  // get results

  const auto& errors = responseTask->getErrorMessages();

  // check for errors

  if (!errors.empty()) {
    std::string errorMsgs;
    for (const auto& msg : errors) {
      errorMsgs += msg + "\n";
    }
    args.GetReturnValue().Set(isolate->ThrowException(OneByteString(isolate, errorMsgs.c_str())));
    return;
  }

  // save performance data of sub-query

  convertPerformanceDataToJson(isolate,
                               responseTask->getPerformanceData(),
                               responseTask->getQueryStart(),
                               isoContext->jsonQueryPerformanceValues);

  // save result tables

  auto resultTable = responseTask->getResultTask()->getResultTable();
  isoContext->tables.push_back(resultTable);

  convertDataflowDataToJson(isolate, isoContext->tables.size() - 1, resultTable->size(), isoContext->jsonQueryDataflow);

  v8::Local<v8::Object> obj;
  wrapTable<hyrise::storage::AbstractTable>(isolate, resultTable, isoContext->tables.size() - 1, obj);
  obj->Set(OneByteString(isolate, "_isModifiable"), v8::Boolean::New(isolate, true));

  args.GetReturnValue().Set(obj);
}

void BuildQuery(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::Local<v8::Object> obj = v8::Object::New(isolate);
  auto operators = v8::Local<v8::Object>::Cast(args[0]);
  auto edges = v8::Local<v8::Array>::Cast(args[1]);

  obj->Set(OneByteString(isolate, "operators"), operators);
  obj->Set(OneByteString(isolate, "edges"), edges);

  args.GetReturnValue().Set(obj);
}

// The goal of the result helpers is to provide the necessary functions to
// present a usable result. This includes basically the following
// possibilities:
//   * create a PC with a pos List
//   * create a new modifiable table based on the old table meta data
//   * create a new modifiable table using the table builder
//   * create a combination as a vertical table
void ScriptOperation::createResultHelpers(v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> global) {
  global->Set(OneByteString(isolate, "createPointerCalculator"),
              v8::FunctionTemplate::New(isolate, createPointerCalculator));
  global->Set(OneByteString(isolate, "copyStructureModifiable"),
              v8::FunctionTemplate::New(isolate, CopyStructureModifiable));
  global->Set(OneByteString(isolate, "buildTableShort"), v8::FunctionTemplate::New(isolate, BuildTableShort));
  global->Set(OneByteString(isolate, "buildTableColumn"), v8::FunctionTemplate::New(isolate, BuildTableColumn));
  global->Set(OneByteString(isolate, "buildTable"), v8::FunctionTemplate::New(isolate, BuildTable));
  global->Set(OneByteString(isolate, "buildVerticalTable"), v8::FunctionTemplate::New(isolate, BuildVerticalTable));
  global->Set(OneByteString(isolate, "include"), v8::FunctionTemplate::New(isolate, Include));
  global->Set(OneByteString(isolate, "log"), v8::FunctionTemplate::New(isolate, LogMessage));
  global->Set(OneByteString(isolate, "executeQuery"), v8::FunctionTemplate::New(isolate, ExecuteQuery));
  global->Set(OneByteString(isolate, "buildQuery"), v8::FunctionTemplate::New(isolate, BuildQuery));
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
    wrapTable<storage::AbstractTable>(isolate, input.getTable(i), i, locals[i]);
    result->Set(i, locals[i]);
  }

  // Return the value through Close.
  return result;
}

#endif

void ScriptOperation::setParameters(const std::vector<std::string>& params) { _parameters = params; }

void ScriptOperation::setScriptSource(const std::string& source) { _scriptSource = source; }

void ScriptOperation::setPapiEvent(const std::string& papiEvent) { _papiEvent = papiEvent; }


Json::Value ScriptOperation::getSubQueryPerformanceData() {
#ifdef WITH_V8
  return _isoContext.jsonQueryPerformanceValues;
#endif
  return Json::Value();
}

Json::Value ScriptOperation::getSubQueryDataflow() {
#ifdef WITH_V8
  return _isoContext.jsonQueryDataflow;
#endif
  return Json::Value();
}

void ScriptOperation::executePlanOperation() {

#ifdef WITH_V8
  // is this necessary?
  const char* flags = "";  //--prof --trace-opt";
  v8::V8::InitializeICU();
  v8::V8::SetFlagsFromString(flags, strlen(flags));

  v8::Isolate* isolate;
  isolate = v8::Isolate::New();
  isolate->Enter();

  // Set the data in the isolate context
  for (const auto& t : input.allOf<storage::AbstractTable>()) {
    _isoContext.tables.push_back(t);
  }
  _isoContext.recordPerformance = true;  //_performance_attr != nullptr; @todo
  _isoContext.papiEvent = getEvent();

  {
    v8::Isolate::Scope isolate_scope(isolate);

    isolate->SetData(0, &_isoContext);

    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);


    // Create a template for the global object.
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

    // Add Helper Functions
    createResultHelpers(isolate, global);

    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);

    // Enter the created context for compiling and
    // running the script.
    v8::Context::Scope context_scope(context);

    // start v8 profiler
    // auto cpuProfiler = isolate->GetCpuProfiler();
    // cpuProfiler->SetSamplingInterval(1000/*us*/);
    // cpuProfiler->StartProfiling(v8::String::NewFromUtf8(isolate, "ScriptOperation"), true);//record_samples=false);

    // Create a string containing the JavaScript source code.
    std::string content;
    if (_scriptSource == "")
      content = readScript(_scriptName);
    else
      content = _scriptSource;

    if (content.size() == 0) {
      throw std::runtime_error("Script is empty, cannot run empty script: " + _scriptName);
    }

    // std::cout << "going to compile/run the following script:\n" << content.c_str() << std::endl;

    v8::Handle<v8::String> source = OneByteString(isolate, content.c_str(), v8::String::kNormalString, content.size());

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

        v8::Handle<v8::Value>* argv = new v8::Handle<v8::Value>[_parameters.size() + 1];

        argv[0] = prepareInputs(isolate);

        for (int i = 1; i < _parameters.size() + 1; i++) {
          argv[i] = v8::JSON::Parse(OneByteString(isolate, _parameters[i - 1].c_str()));
        }

        v8::Local<v8::Object> result;

        try {
          result = v8::Local<v8::Object>::Cast(fun->Call(context->Global(), _parameters.size() + 1, argv));
        }
        catch (...) {
          std::exception_ptr p = std::current_exception();
          std::cout << "scriptop:" << (p ? p.__cxa_exception_type()->name() : "null") << std::endl;
        }

        delete[] argv;

        if (result.IsEmpty()) {
          std::string errorMsg = *v8::String::Utf8Value(trycatch.Exception());
          v8::Handle<v8::Message> message = trycatch.Message();
          if (!message.IsEmpty()) {
            int linenum = message->GetLineNumber();
            if (_scriptName != "")
              errorMsg.insert(0, "error in script <" + _scriptName + ".js:" + std::to_string(linenum) + "> ");
            else
              errorMsg.insert(0, "error in script line " + std::to_string(linenum) + "> ");
          }
          throw std::runtime_error(errorMsg);
        }

        // Unwrap the data and extract the shared_ptr for the result
        if (!result->IsUndefined()) {
          size_t internal = result->Get(OneByteString(isolate, "_internalId"))->Uint32Value();
          addResult(_isoContext.tables[internal]);
        }
      }
    }

    // auto cpuProfile = cpuProfiler->StopProfiling(v8::String::NewFromUtf8(isolate, "ScriptOperation"));
    // std::cout << parseCpuProfile(cpuProfile) << std::endl;
    // cpuProfile->Delete();
  }

  // Dispose the persistent context.
  isolate->Exit();
  isolate->Dispose();
#endif
}

const PlanOperation* ScriptOperation::execute() {
  const bool recordPerformance = _performance_attr != nullptr;

  // Check if we really need this
  epoch_t startTime = 0;
  if (recordPerformance)
    startTime = get_epoch_nanoseconds();

  PapiTracer pt;

  // Start the execution
  refreshInput();
  setupPlanOperation();

  // if (recordPerformance) {
  //   pt.addEvent("PAPI_TOT_CYC");
  //   pt.addEvent(getEvent());
  //   pt.start();
  // }

  executePlanOperation();

  // if (recordPerformance)
  //   pt.stop();

  teardownPlanOperation();

  if (recordPerformance) {
    epoch_t endTime = get_epoch_nanoseconds();
    std::string threadId = boost::lexical_cast<std::string>(std::this_thread::get_id());

    int64_t duration = endTime - startTime;

    *_performance_attr = (performance_attributes_t) {
        duration,            0 /*pt.value("PAPI_TOT_CYC"), pt.value(getEvent())*/, "NO_PAPI_duration_in_ns",
        planOperationName(), _operatorId,                                          startTime,
        endTime,             threadId,                                             std::numeric_limits<size_t>::max()};
  }

  setState(OpSuccess);
  return this;
}

std::shared_ptr<PlanOperation> ScriptOperation::parse(const Json::Value& data) {
  auto op = std::make_shared<ScriptOperation>();
  op->setScriptName(data["script"].asString());

  return op;
}
}
}
