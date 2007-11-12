/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "CheckIntegrityCommand.h"
#include "CheckIntegrityMan.h"
#include "CheckIntegrityEntry.h"
#include "RequestGroup.h"
#include "FileAllocationEntry.h"
#include "InitiateConnectionCommandFactory.h"
#include "DlAbortEx.h"
#include "message.h"
#include "DownloadCommand.h"
#include "prefs.h"

CheckIntegrityCommand::CheckIntegrityCommand(int32_t cuid, RequestGroup* requestGroup, DownloadEngine* e, const CheckIntegrityEntryHandle& entry):
  RealtimeCommand(cuid, requestGroup, e),
  _entry(entry)
{
  _e->_checkIntegrityMan->addCheckIntegrityEntry(_entry);
}

CheckIntegrityCommand::~CheckIntegrityCommand()
{
  _e->_checkIntegrityMan->removeCheckIntegrityEntry(_entry);
}

bool CheckIntegrityCommand::executeInternal()
{
  if(_requestGroup->isHaltRequested()) {
    return true;
  }
  _entry->validateChunk();
  if(_entry->finished()) {
    if(_requestGroup->downloadFinished()) {
      logger->notice(MSG_VERIFICATION_SUCCESSFUL,
		     _requestGroup->getFilePath().c_str());
      _e->addCommand(_entry->onDownloadFinished(_e));
    } else {
      logger->error(MSG_VERIFICATION_FAILED,
		    _requestGroup->getFilePath().c_str());
      _e->addCommand(_entry->onDownloadIncomplete(_e));
    }
    return true;
  } else {
    _e->commands.push_back(this);
    return false;
  }
}

bool CheckIntegrityCommand::handleException(Exception* e)
{
  logger->error(MSG_FILE_VALIDATION_FAILURE, e, cuid);
  logger->error(MSG_DOWNLOAD_NOT_COMPLETE, cuid, _requestGroup->getFilePath().c_str());
  return true;
}
