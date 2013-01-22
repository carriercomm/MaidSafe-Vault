/*******************************************************************************
*  Copyright 2012 maidsafe.net limited                                        *
*                                                                             *
*  The following source code is property of maidsafe.net limited and is not   *
*  meant for external use.  The use of this code is governed by the licence   *
*  file licence.txt found in the root of this directory and also on           *
*  www.maidsafe.net.                                                          *
*                                                                             *
*  You are not free to copy, amend or otherwise use this source code without  *
*  the explicit written permission of the board of directors of maidsafe.net. *
******************************************************************************/

#include "maidsafe/vault/disk_based_storage.h"

#include <memory>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/test.h"

#include "maidsafe/passport/types.h"

#include "maidsafe/nfs/message.h"

#include "maidsafe/vault/utils.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

namespace test {

template <typename T>
class DiskStorageTest : public testing::Test {
 public:
  DiskStorageTest()
      : root_directory_(maidsafe::test::CreateTestPath("MaidSafe_Test_DiskStorage")) {}

 protected:
  maidsafe::test::TestPath root_directory_;

  std::string GenerateFileContent(uint32_t max_file_size) {
    protobuf::DiskStoredFile disk_file;
    protobuf::DiskStoredElement disk_element;
    disk_element.set_data_name(RandomString(64));
    disk_element.set_version(RandomUint32() % 100);
    disk_element.set_serialised_value(RandomString(RandomUint32() % max_file_size));
    disk_file.add_disk_element()->CopyFrom(disk_element);
    return disk_file.SerializeAsString();
  }

  void ChangeDiskElement(std::string& serialised_disk_element,
                         const std::string& new_serialised_value) {
    protobuf::DiskStoredElement disk_element;
    assert(disk_element.ParseFromString(serialised_disk_element) &&
           "Received element doesn't parse.");
    disk_element.set_serialised_value(new_serialised_value);
    serialised_disk_element = disk_element.SerializeAsString();
  }
};

typedef testing::Types<passport::PublicAnmid,
                       passport::PublicAnsmid,
                       passport::PublicAntmid,
                       passport::PublicAnmaid,
                       passport::PublicMaid,
                       passport::PublicPmid,
                       passport::Mid,
                       passport::Smid,
                       passport::Tmid,
                       passport::PublicAnmpid,
                       passport::PublicMpid> AllTypes;

TYPED_TEST_CASE(DiskStorageTest, AllTypes);

TYPED_TEST(DiskStorageTest, BEH_ConstructorDestructor) {
  fs::path root_path(*(this->root_directory_) / RandomString(6));
  boost::system::error_code error_code;
  EXPECT_FALSE(fs::exists(root_path, error_code));
  {
    DiskBasedStorage disk_based_storage(root_path);
    EXPECT_TRUE(fs::exists(root_path, error_code));
    // An empty file shall be generated in constructor
    std::future<uint32_t> file_count(disk_based_storage.GetFileCount());
    EXPECT_EQ(file_count.get(), 1);
  }
  EXPECT_TRUE(fs::exists(root_path, error_code));
}

TYPED_TEST(DiskStorageTest, BEH_FileHandlers) {
  fs::path root_path(*(this->root_directory_) / RandomString(6));
  DiskBasedStorage disk_based_storage(root_path);
  std::map<fs::path, NonEmptyString> files;
  uint32_t num_files(100), max_file_size(10000);
  std::vector<uint32_t> file_numbers;
  for (uint32_t i(0); i < num_files; ++i)
    file_numbers.push_back(i);
  std::random_shuffle(file_numbers.begin(), file_numbers.end());

  for (uint32_t i(0); i < num_files; ++i) {
    NonEmptyString file_content(this->GenerateFileContent(max_file_size));
    std::string hash(EncodeToBase32(crypto::Hash<crypto::SHA512>(file_content)));
    std::string file_name(std::to_string(file_numbers[i]) + "." + hash);
    fs::path file_path(root_path / file_name);
    files.insert(std::make_pair(file_path, file_content));
    disk_based_storage.PutFile(file_path, file_content);
  }

  std::future<uint32_t> file_count(disk_based_storage.GetFileCount());
  EXPECT_EQ(file_count.get(), num_files);
  std::future<DiskBasedStorage::PathVector> file_paths = disk_based_storage.GetFileNames();
  EXPECT_EQ(file_paths.get().size(), num_files);

  boost::system::error_code error_code;
  auto itr = files.begin();
  do {
    fs::path path((*itr).first);
    EXPECT_TRUE(fs::exists(path, error_code));

    auto result = disk_based_storage.GetFile(path);
    NonEmptyString content(result.get());
    EXPECT_EQ(content, (*itr).second);

    ++itr;
  } while (itr != files.end());
}

TYPED_TEST(DiskStorageTest, BEH_FileHandlersWithCorruptingThread) {
  // File handlers of DiskBasedStorage are non-blocking, using a separate Active object
  Active active;
  fs::path root_path(*(this->root_directory_) / RandomString(6));
  DiskBasedStorage disk_based_storage(root_path);
  std::map<fs::path, NonEmptyString> files;
  uint32_t num_files(10), max_file_size(10000);
  for (uint32_t i(0); i < num_files; ++i) {
    NonEmptyString file_content(this->GenerateFileContent(max_file_size));
    std::string hash(EncodeToBase32(crypto::Hash<crypto::SHA512>(file_content)));
    std::string file_name(std::to_string(i) + "." + hash);
    fs::path file_path(root_path / file_name);
    files.insert(std::make_pair(file_path, file_content));
  }

  for (auto itr(files.begin()); itr != files.end(); ++itr) {
    fs::path file_path((*itr).first);
    active.Send([file_path] () {
                  maidsafe::WriteFile(file_path, RandomString(100));
                });
    disk_based_storage.PutFile(file_path, (*itr).second);
  }

  std::future<uint32_t> file_count(disk_based_storage.GetFileCount());
  EXPECT_EQ(file_count.get(), num_files);

//   Active active_delete;
  boost::system::error_code error_code;
  auto itr = files.begin();
  do {
    fs::path path((*itr).first);
    EXPECT_TRUE(fs::exists(path, error_code));

    active.Send([&disk_based_storage, path] () {
                  auto result = disk_based_storage.GetFile(path);
                  do {
                    Sleep(boost::posix_time::milliseconds(1));
                  } while (!result.valid());
                  EXPECT_FALSE(result.has_exception()) << "Get exception when trying to get "
                                                       << path.filename();
                  if (!result.has_exception()) {
                    NonEmptyString content(result.get());
                    EXPECT_TRUE(content.IsInitialised());
                  }
                });
//     active_delete.Send([path] () { fs::remove(path); });

    ++itr;
  } while (itr != files.end());
}

TYPED_TEST(DiskStorageTest, BEH_ElementHandlers) {
  fs::path root_path(*(this->root_directory_) / RandomString(6));
  DiskBasedStorage disk_based_storage(root_path);

  typename TypeParam::name_type name((Identity(RandomString(crypto::SHA512::DIGESTSIZE))));
  int32_t version(RandomUint32());
  std::string serialised_value(RandomString(10000));
  disk_based_storage.Store<TypeParam>(name, version, serialised_value);

  protobuf::DiskStoredElement element;
  element.set_data_name(name.data.string());
  element.set_version(version);
  element.set_serialised_value(serialised_value);
  protobuf::DiskStoredFile disk_file;
  disk_file.add_disk_element()->CopyFrom(element);
  std::string hash = EncodeToBase32(crypto::Hash<crypto::SHA512>(disk_file.SerializeAsString()));
  fs::path file_path = maidsafe::vault::detail::GetFilePath(
      root_path, hash, disk_based_storage.GetFileCount().get() - 1);

  Sleep(boost::posix_time::milliseconds(10));
  boost::system::error_code error_code;
  EXPECT_TRUE(fs::exists(file_path, error_code));
  {
    auto result = disk_based_storage.GetFile(file_path);
    NonEmptyString fetched_content = result.get();
    EXPECT_EQ(fetched_content.string(), disk_file.SerializeAsString());
  }

  std::string new_serialised_value(RandomString(10000));
  element.set_serialised_value(new_serialised_value);
  disk_file.clear_disk_element();
  disk_file.add_disk_element()->CopyFrom(element);

  disk_based_storage.Modify<TypeParam>(name, version,
                                       [this, new_serialised_value]
                                          (std::string& serialised_disk_element) {
                                         this->ChangeDiskElement(serialised_disk_element,
                                                                 new_serialised_value); },
                                       serialised_value);

  std::string new_hash = EncodeToBase32(
                            crypto::Hash<crypto::SHA512>(disk_file.SerializeAsString()));
  fs::path new_file_path = maidsafe::vault::detail::GetFilePath(
      root_path, new_hash, disk_based_storage.GetFileCount().get() - 1);
  {
    auto result = disk_based_storage.GetFile(new_file_path);
    NonEmptyString fetched_content = result.get();
    EXPECT_EQ(fetched_content.string(), disk_file.SerializeAsString());
    EXPECT_FALSE(fs::exists(file_path, error_code));
    EXPECT_TRUE(fs::exists(new_file_path, error_code));
  }

  disk_based_storage.Delete<TypeParam>(name, version);
  Sleep(boost::posix_time::milliseconds(10));
  EXPECT_FALSE(fs::exists(new_file_path, error_code));
}

TYPED_TEST(DiskStorageTest, BEH_ElementHandlersWithMultThreads) {
  fs::path root_path(*(this->root_directory_) / RandomString(6));
  DiskBasedStorage disk_based_storage(root_path);

  std::vector<std::shared_ptr<Active>> active_list;
  std::map<std::pair<std::string, int32_t>, std::string> element_list;
  uint32_t num_files(10), max_file_size(10000);
  for (uint32_t i(0); i < num_files; ++i) {
    std::shared_ptr<Active> active_ptr;
    active_ptr.reset(new Active());
    active_list.push_back(active_ptr);

    std::string file_name(RandomString(crypto::SHA512::DIGESTSIZE));
    typename TypeParam::name_type name((Identity(file_name)));
    int32_t version(RandomUint32());
    std::string serialised_value(RandomString(max_file_size));
    element_list.insert(std::make_pair(std::make_pair(file_name, version),
                                       serialised_value));
    disk_based_storage.Store<TypeParam>(name, version, serialised_value);
  }

  // Check previous stored element does exist and contained content is correct
  boost::system::error_code error_code;
  auto itr = element_list.begin();
  size_t i(0);
  do {
    std::string file_name = (*itr).first.first;
    std::string serialised_value = (*itr).second;
    EXPECT_TRUE(fs::exists(root_path / file_name, error_code));
    active_list[i]->Send([&disk_based_storage,
                          root_path, file_name, serialised_value] () {
                            auto result = disk_based_storage.GetFile(root_path / file_name);
                            NonEmptyString fetched_content = result.get();
                            EXPECT_EQ(fetched_content.string(), serialised_value);
                          });
    ++itr;
    ++i;
  } while (itr != element_list.end());

  // Generate new content for each element
  itr = element_list.begin();
  do {
    (*itr).second = RandomString(max_file_size);
    ++itr;
  } while (itr != element_list.end());

  // Modify each element's content parallel
  itr = element_list.begin();
  i = 0;
  do {
    std::string file_name = (*itr).first.first;
    typename TypeParam::name_type name((Identity(file_name)));
    int32_t version = (*itr).first.second;
    std::string new_serialised_value = (*itr).second;
    active_list[i]->Send([&disk_based_storage,
                          name, version, new_serialised_value] () {
                            disk_based_storage.Modify<TypeParam>(name, version, nullptr,
                                                                 new_serialised_value);
                          });
    ++itr;
    ++i;
  } while (itr != element_list.end());

  // Parallel Verify each element's content has been properly updated
  itr = element_list.begin();
  i = 0;
  do {
    std::string file_name = (*itr).first.first;
    std::string new_serialised_value = (*itr).second;
    active_list[i]->Send([&disk_based_storage,
                          root_path, file_name, new_serialised_value] () {
                            auto result = disk_based_storage.GetFile(root_path / file_name);
                            NonEmptyString fetched_content = result.get();
                            EXPECT_EQ(fetched_content.string(), new_serialised_value);
                          });
    ++itr;
    ++i;
  } while (itr != element_list.end());

  // Parallel delete all elements
  itr = element_list.begin();
  i = 0;
  do {
    std::string file_name = (*itr).first.first;
    typename TypeParam::name_type name((Identity(file_name)));
    int32_t version = (*itr).first.second;
    active_list[i]->Send([&disk_based_storage, name, version] () {
                            disk_based_storage.Delete<TypeParam>(name, version);
                          });
    ++itr;
    ++i;
  } while (itr != element_list.end());

  itr = element_list.begin();
  do {
    std::string file_name = (*itr).first.first;
    EXPECT_FALSE(fs::exists(root_path / file_name, error_code));
    ++itr;
  } while (itr != element_list.end());
}

}  // namespace test

}  // namespace vault

}  // namespace maidsafe
