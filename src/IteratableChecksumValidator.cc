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
#include "IteratableChecksumValidator.h"
#include "Util.h"
#include "message.h"
#include "SingleFileDownloadContext.h"
#include "PieceStorage.h"
#include "messageDigest.h"
#include "LogFactory.h"
#include "Logger.h"
#include "DiskAdaptor.h"
#include "BitfieldMan.h"

#define BUFSIZE 16*1024

IteratableChecksumValidator::IteratableChecksumValidator(const SingleFileDownloadContextHandle& dctx,
							 const PieceStorageHandle& pieceStorage):
  _dctx(dctx),
  _pieceStorage(pieceStorage),
  _currentOffset(0),
  _ctx(0),
  _logger(LogFactory::getInstance()) {}

IteratableChecksumValidator::~IteratableChecksumValidator() {}

void IteratableChecksumValidator::validateChunk()
{
  if(!finished()) {
    unsigned char data[BUFSIZE];
    int32_t length = _pieceStorage->getDiskAdaptor()->readData(data, sizeof(data), _currentOffset);
    _ctx->digestUpdate(data, length);
    _currentOffset += length;
    if(finished()) {
      string actualChecksum = Util::toHex((const unsigned char*)_ctx->digestFinal().c_str(), _ctx->digestLength());
      if(_dctx->getChecksum() == actualChecksum) {
	_pieceStorage->markAllPiecesDone();
      } else {
	BitfieldMan bitfield(_dctx->getPieceLength(), _dctx->getTotalLength());
	_pieceStorage->setBitfield(bitfield.getBitfield(), bitfield.getBitfieldLength());
      }
    }
  }
}

bool IteratableChecksumValidator::finished() const
{
  return _currentOffset >= _dctx->getTotalLength();
}

int64_t IteratableChecksumValidator::getTotalLength() const
{
  return _dctx->getTotalLength();
}

void IteratableChecksumValidator::init()
{
  _currentOffset = 0;
  _ctx = new MessageDigestContext();
  _ctx->trySetAlgo(_dctx->getChecksumHashAlgo());
  _ctx->digestInit();
}
