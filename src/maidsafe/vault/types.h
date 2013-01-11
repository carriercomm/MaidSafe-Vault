/***************************************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#ifndef MAIDSAFE_VAULT_TYPES_H_
#define MAIDSAFE_VAULT_TYPES_H_

#include "maidsafe/common/tagged_value.h"
#include "maidsafe/common/types.h"

#include "maidsafe/nfs/nfs.h"


namespace maidsafe {

namespace vault {

typedef TaggedValue<Identity, struct VaultIdentityTag> AccountName;

typedef nfs::NetworkFileSystem<GetFromMaidAccountHolder<PersonaType::kMaidAccountHolder>,
                          PutToMetadataManager,
                          PostSynchronisation<PersonaType::kMaidAccountHolder>,
                          DeleteFromMetadataManager> MaidAccountHolderNfs;

typedef nfs::NetworkFileSystem<GetFromPmidAccountHolder,
                          PutToPmidAccountHolder,
                          NoPost<passport::Pmid>,
                          DeleteFromPmidAccountHolder> MetadataManagerNfs;


}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_TYPES_H_
