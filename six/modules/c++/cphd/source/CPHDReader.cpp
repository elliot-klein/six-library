/* =========================================================================
 * This file is part of cphd-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2019, MDA Information Systems LLC
 *
 * cphd-c++ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */
#include <sys/Conf.h>
#include <except/Exception.h>
#include <io/StringStream.h>
#include <io/FileInputStream.h>
#include <logging/NullLogger.h>
#include <mem/ScopedArray.h>
#include <xml/lite/MinidomParser.h>
#include <cphd/CPHDReader.h>
#include <cphd/CPHDXMLControl.h>

namespace cphd
{
CPHDReader::CPHDReader(std::shared_ptr<io::SeekableInputStream> inStream,
                       size_t numThreads,
                       const std::vector<std::string>& schemaPaths,
                       std::shared_ptr<logging::Logger> logger)
{
    initialize(inStream, numThreads, logger, schemaPaths);
}

CPHDReader::CPHDReader(const std::string& fromFile,
                       size_t numThreads,
                       const std::vector<std::string>& schemaPaths,
                       std::shared_ptr<logging::Logger> logger)
{
    initialize(std::shared_ptr<io::SeekableInputStream>(
        new io::FileInputStream(fromFile)), numThreads, logger, schemaPaths);
}

void CPHDReader::initialize(std::shared_ptr<io::SeekableInputStream> inStream,
                            size_t numThreads,
                            std::shared_ptr<logging::Logger> logger,
                            const std::vector<std::string>& schemaPaths)
{
    mFileHeader.read(*inStream);

    // Read in the XML string
    inStream->seek(mFileHeader.getXMLBlockByteOffset(), io::Seekable::START);

    xml::lite::MinidomParser xmlParser;
    xmlParser.preserveCharacterData(true);
    xmlParser.parse(*inStream, mFileHeader.getXMLBlockSize());

    if (logger.get() == NULL)
    {
        logger.reset(new logging::NullLogger());
    }

    mMetadata = CPHDXMLControl(logger.get(), false).fromXML(xmlParser.getDocument(), schemaPaths);

    mSupportBlock.reset(new SupportBlock(inStream, mMetadata->data,
                        mFileHeader.getSupportBlockByteOffset(),
                        mFileHeader.getSupportBlockSize()));

    // Load the PVPBlock into memory
    mPVPBlock.reset(new PVPBlock(mMetadata->pvp, mMetadata->data));
    mPVPBlock->load(*inStream,
                    mFileHeader.getPvpBlockByteOffset(),
                    mFileHeader.getPvpBlockSize(),
                    numThreads);

    // Setup for wideband reading
    mWideband.reset(new Wideband(inStream, *mMetadata,
                                 mFileHeader.getSignalBlockByteOffset(),
                                 mFileHeader.getSignalBlockSize()));
}
}
