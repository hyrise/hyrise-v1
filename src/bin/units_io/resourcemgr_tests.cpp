// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include <memory>

#include "testing/test.h"

#include <io/shortcuts.h>
#include <io/ResourceManager.h>
#include <storage/AbstractResource.h>
#include <storage/AbstractIndex.h>
#include <storage/Table.h>

namespace hyrise { namespace io {

namespace {
  storage::aresource_ptr_t emptyResource() {
    return std::make_shared<storage::AbstractResource>();
  }

  storage::atable_ptr_t emptyTable() {
    auto metadata = std::make_shared<storage::metadata_list>();
    return std::make_shared<storage::Table>(metadata.get());
  }

  class FakeIndex : public storage::AbstractIndex {
   public:
    void shrink() {}
  };

  storage::aindex_ptr_t emptyIndex() {
    return std::make_shared<FakeIndex>();
  }
} // namespace


class ResourceManagerTests : public Test {

public:
  ResourceManagerTests() {
    rm = &ResourceManager::getInstance();
  }

  virtual void SetUp() {
    rm->clear();
  }
  
  ResourceManager *rm;
};

TEST_F(ResourceManagerTests, is_singleton) {
  ASSERT_TRUE(rm != nullptr);

  auto rm2 = &ResourceManager::getInstance();

  ASSERT_EQ(rm, rm2);
}

TEST_F(ResourceManagerTests, clear_resources) {
  EXPECT_EQ(0u, rm->size());

  rm->add("TestResource1", emptyResource());
  rm->add("TestResource2", emptyResource());
  EXPECT_EQ(2u, rm->size());

  rm->clear();
  EXPECT_EQ(0u, rm->size());
}

TEST_F(ResourceManagerTests, add_increase_size) {
  ASSERT_EQ(0u, rm->size());
  rm->add("1", emptyResource());
  ASSERT_EQ(1u, rm->size());
  rm->add("2", emptyTable());
  ASSERT_EQ(2u, rm->size());
  rm->add("3", emptyIndex());
  ASSERT_EQ(3u, rm->size());
}

TEST_F(ResourceManagerTests, added_resources_exist) {
  EXPECT_FALSE(rm->exists("Resource1"));
  EXPECT_FALSE(rm->exists("Resource2"));
  
  rm->add("Resource1", emptyResource());
  EXPECT_TRUE(rm->exists("Resource1")); 
  EXPECT_FALSE(rm->exists("Resource2")); 

  rm->add("Resource2", emptyResource());
  EXPECT_TRUE(rm->exists("Resource1")); 
  EXPECT_TRUE(rm->exists("Resource2")); 
}

TEST_F(ResourceManagerTests, assure_exists_throws_exception) {
  EXPECT_THROW(rm->assureExists("Resource"), ResourceNotExistsException);
  rm->add("Resource", emptyResource());
  EXPECT_NO_THROW(rm->assureExists("Resource"));
}

TEST_F(ResourceManagerTests, add_and_get_equal_resources) {
  const auto resource1 = emptyResource();
  const auto resource2 = emptyTable();
  ASSERT_NE(resource1, resource2);

  rm->add("Resource1", resource1);
  rm->add("Resource2", resource2);

  EXPECT_EQ(resource2, rm->getResource("Resource2"));
  EXPECT_EQ(resource1, rm->getResource("Resource1"));
  EXPECT_NE(resource1, rm->getResource("Resource2"));
  EXPECT_NE(resource2, rm->getResource("Resource1"));
}

TEST_F(ResourceManagerTests, add_throws_exception) {
  EXPECT_NO_THROW(rm->add("Resource", emptyResource()));
  EXPECT_THROW(rm->add("Resource", emptyResource()), ResourceAlreadyExistsException);
}

TEST_F(ResourceManagerTests, get_throws_exception) {
  EXPECT_THROW(rm->getResource("Resource"), ResourceNotExistsException);
  rm->add("Resource", emptyResource());
  EXPECT_NO_THROW(rm->getResource("Resource"));
}

TEST_F(ResourceManagerTests, remove_decrease_size) {
  rm->add("Resource1", emptyResource());
  rm->add("Resource2", emptyTable());
  rm->add("Resource3", emptyIndex());
  ASSERT_EQ(3u, rm->size());

  rm->remove("Resource1");
  ASSERT_EQ(2u, rm->size());
  rm->remove("Resource3");
  ASSERT_EQ(1u, rm->size());
  rm->remove("Resource2");
  ASSERT_EQ(0u, rm->size());
}

TEST_F(ResourceManagerTests, removed_resources_do_not_exists) {
  rm->add("Resource1", emptyResource());
  rm->add("Resource2", emptyResource());

  rm->remove("Resource1");
  EXPECT_FALSE(rm->exists("Resource1"));
  EXPECT_TRUE(rm->exists("Resource2"));
  
  rm->remove("Resource2");
  EXPECT_FALSE(rm->exists("Resource1"));
  EXPECT_FALSE(rm->exists("Resource2"));
}

TEST_F(ResourceManagerTests, remove_throws_exception) {
  EXPECT_THROW(rm->remove("Resource"), ResourceNotExistsException);
  rm->add("Resource", emptyResource());
  EXPECT_NO_THROW(rm->remove("Resource"));
}

TEST_F(ResourceManagerTests, multiple_add_and_remove) {
  const auto resource = emptyResource();

  rm->add("Resource", resource);
  rm->remove("Resource");
  EXPECT_NO_THROW(rm->add("Resource", resource));
  EXPECT_NO_THROW(rm->remove("Resource"));
}

TEST_F(ResourceManagerTests, replace_resource) {
  const auto resource1 = emptyResource();
  const auto resource2 = emptyResource();

  rm->add("Resource", resource1);
  EXPECT_EQ(resource1, rm->getResource("Resource"));
  rm->replace("Resource", resource2);
  EXPECT_EQ(resource2, rm->getResource("Resource"));
  EXPECT_NE(resource1, rm->getResource("Resource"));
}

TEST_F(ResourceManagerTests, replace_throws_exception) {
  const auto newResource = emptyResource();
  EXPECT_THROW(rm->replace("Resource", newResource), ResourceNotExistsException);
}

TEST_F(ResourceManagerTests, multiple_contexts) {
  const auto resource1 = emptyResource();
  const auto resource2 = emptyResource();
  
  { //add
    auto rm = &ResourceManager::getInstance();
    rm->add("Resource", resource1);
  }

  { //get
    auto rm = &ResourceManager::getInstance();
    ASSERT_EQ(resource1, rm->getResource("Resource"));
  }

  { //replace
    auto rm = &ResourceManager::getInstance();
    rm->replace("Resource", resource2);
  }

  { //remove
    ASSERT_EQ(resource2, rm->getResource("Resource"));
    auto rm = &ResourceManager::getInstance();
    rm->remove("Resource");
  }

  {//test
    auto rm = &ResourceManager::getInstance();
    ASSERT_FALSE(rm->exists("Resource"));
  }
}

TEST_F(ResourceManagerTests, get_typed_resource) { 
  const auto resource = emptyResource();
  const auto table = emptyTable();
  const auto index = emptyIndex();
 
  rm->add("Resource", resource);
  rm->add("Table", table);
  rm->add("Index", index);

  const storage::aresource_ptr_t resource_g = rm->get<storage::AbstractResource>("Resource");
  EXPECT_EQ(resource, resource_g);

  const auto table_g = rm->get<storage::AbstractTable>("Table");
  EXPECT_EQ(table, table_g);

  const storage::aindex_ptr_t index_g = rm->get<storage::AbstractIndex>("Index");
  EXPECT_EQ(index, index_g);
}

TEST_F(ResourceManagerTests, get_typed_resource_throws_exception) {
  rm->add("Resource", emptyResource());
  rm->add("Table", emptyTable());
  rm->add("Index", emptyIndex());

  EXPECT_NO_THROW(rm->get<storage::AbstractResource>("Resource"));
  EXPECT_NO_THROW(rm->get<storage::AbstractTable>("Table"));
  EXPECT_NO_THROW(rm->get<storage::AbstractIndex>("Index"));
  
  EXPECT_THROW(   rm->get<storage::AbstractTable>("Resource"), std::runtime_error);
  EXPECT_THROW(   rm->get<storage::AbstractIndex>("Resource"), std::runtime_error);

  EXPECT_NO_THROW(rm->get<storage::AbstractResource>("Table"));
  EXPECT_THROW(   rm->get<storage::AbstractIndex>("Table"), std::runtime_error);

  EXPECT_NO_THROW(rm->get<storage::AbstractResource>("Index"));
  EXPECT_THROW(   rm->get<storage::AbstractTable>("Index"), std::runtime_error);
}

} } // namespace hyrise::io

