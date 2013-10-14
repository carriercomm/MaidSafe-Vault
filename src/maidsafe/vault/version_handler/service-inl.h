/*  Copyright 2013 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_VAULT_VERSION_HANDLER_SERVICE_INL_H_
#define MAIDSAFE_VAULT_VERSION_HANDLER_SERVICE_INL_H_

#include <exception>
#include <string>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/data_types/data_name_variant.h"
#include "maidsafe/nfs/utils.h"
#include "maidsafe/nfs/types.h"
#include "maidsafe/vault/unresolved_action.h"
#include "maidsafe/vault/utils.h"

namespace maidsafe {

namespace vault {

namespace detail {

// VersionHandlerUnresolvedEntry UnresolvedEntryFromMessage(const nfs::Message& message);

}  // namespace detail

// template<typename Data>
// void VersionHandlerService::HandleMessage(const nfs::Message& message,
//                                                  const routing::ReplyFunctor& reply_functor) {
// //   ValidateSender(message);

//  nfs::Reply reply(CommonErrors::pending_result);
//  {
//    std::lock_guard<std::mutex> lock(accumulator_mutex_);
//    if (accumulator_.CheckHandled(message, reply))
//      return reply_functor(reply.Serialise()->string());
//  }

//   if (message.data().action == nfs::MessageAction::kSynchronise)
//      return HandleSynchronise(message);   // No accumulate
//     database_merge_.insert(message);
//   if (message.data().action == nfs::MessageAction::kAccountTransfer)
//     database_merge_.insert(message);

//   if (message.data().action == nfs::MessageAction::kGet)
//     return HandleGet(message, reply_functor);  //  Add to accumulator on action
//   if (message.data().action == nfs::MessageAction::kGetBranch)
//     return HandleGetBranch(message, reply_functor);  //  Add to accumulator on action

//   if (message.data().action != nfs::MessageAction::kPut &&
//       message.data().action != nfs::MessageAction::kDeleteBranchUntilFork &&
//       message.data().action != nfs::MessageAction::kDelete &&
//       message.data().action != nfs::MessageAction::kAccountTransfer)
//     ThrowError(CommonErrors::invalid_parameter);

//   // accumulate then action, on completion then set reply
//   std::lock_guard<std::mutex> lock(accumulator_mutex_);
//   if (!(accumulator_.PushSingleResult(
//         message,
//         reply_functor,
//         nfs::Reply(maidsafe_error(CommonErrors::pending_result))).size() <
//         routing::Parameters::node_group_size -1U)) {
//     Synchronise<Data>(message);
//   }
//}

//// In this persona we sync all mutating actions, on sucess the reply_functor is fired (if
///available)

// template<typename Data>
// void VersionHandlerService::Synchronise(const nfs::Message& message) {
//  auto entry =  detail::UnresolvedEntryFromMessage(message);
//  nfs_.Sync(DataNameVariant(Data::Name(message.data().name)), entry.Serialise().data);  // does
// not include
//                                                                            // original_message_id
//  entry.original_message_id = message.message_id();
//  entry.source_node_id = message.source().node_id; // with data().originator_id we can
//                                                    // recover the accumulated requests in
//                                                    // HandleSync
//  std::lock_guard<std::mutex> lock(sync_mutex_);
//  sync_.AddUnresolvedEntry(entry);
//}

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_VERSION_HANDLER_SERVICE_INL_H_