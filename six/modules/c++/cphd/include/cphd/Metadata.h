/* =========================================================================
 * This file is part of cphd-c++
 * =========================================================================
 *
 * (C) Copyright 2004 - 2018, MDA Information Systems LLC
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


#ifndef __CPHD_METADATA_H__
#define __CPHD_METADATA_H__

#include <ostream>

#include <cphd/CollectionID.h>
#include <cphd/Channel.h>
#include <cphd/Data.h>
#include <cphd/Global.h>
#include <cphd/SceneCoordinates.h>
#include <cphd/PVP.h>
#include <cphd/Dwell.h>
#include <cphd/ReferenceGeometry.h>

#include <cphd/SupportArray.h>
#include <cphd/Antenna.h>
#include <cphd/TxRcv.h>
#include <cphd/ErrorParameters.h>
#include <cphd/ProductInfo.h>
#include <cphd/MatchInfo.h>
#include <cphd/GeoInfo.h>

namespace cphd
{
/*!
 *  \class Metadata
 *
 *  This class extends the Data object providing the CPHD
 *  layer and utilities for access.  In order to write a CPHD,
 *  you must have a CPHDData object that is populated with
 *  the mandatory parameters, and you must add it to the Container
 *  object first.
 *
 *  This object contains all of the sub-blocks for CPHD.
 *
 *
 */
struct Metadata
{
    Metadata()
    {
    }

    //!  CollectionInfo block.  Contains the general collection information
    //!  CPHD can use the SICD Collection Information block directly
    CollectionID collectionID;

    //!  Global block. Contains the information
    Global global;

    //! Scene Coordinates block. Parameters that define geographic
    //! coordinates for in the imaged scene
    SceneCoordinates sceneCoordinates;

    //!  Data block. Very unfortunate name, but matches the CPHD spec.
    //!  Contains parameters that describe binary data components contained
    //!  in the product
    Data data;

    //! Channel block. Parameters that describe the data channels contained
    //! in the product
    Channel channel;

    //! PVP block. Parameters that describe the size of position of each
    //! vector paramter
    Pvp pvp;

    //! Dwell block. Paramaters that specify the dwell time supported by
    //! the signal arrays contained in the CPHD product
    Dwell dwell;

    //! Reference Geometry block. Parameter describes the collection geometry
    //! for the reference vector (v_CH_REF) of the reference channel (REF_CH_ID)
    ReferenceGeometry referenceGeometry;

    //! (Optional) SupportArray block. Describes the binary support array 
    //! content and grid coordinates
    mem::ScopedCopyablePtr<SupportArray> supportArray;

    //! (optional) Atenna block. Describes the trasmit and receive antennas
    mem::ScopedCopyablePtr<Antenna> antenna;

    //! (Optional) TxRcv block. Describes the transmitted waveform(s) and 
    //! receiver configurations used in the collection.
    mem::ScopedCopyablePtr<TxRcv> txRcv;

    //! (Optional) Error Parameters block. Describes the statistics of errors
    //! in measured or estimated parameters that describe the collection
    mem::ScopedCopyablePtr<ErrorParameters> errorParameters;

    //! (Optional) Product Information block. General information about the CPHD 
    // product or derived products created from it
    mem::ScopedCopyablePtr<ProductInfo> productInfo;

    //! (Optional) Geograpy Information block. Describes geographic features
    std::vector<GeoInfo> geoInfo;

    //! (Optional) Match Information block. Information about other collections
    //! that are matched to the collection generated by this CPHD product
    mem::ScopedCopyablePtr<MatchInfo> matchInfo;

    bool operator==(const Metadata& other) const;

    bool operator!=(const Metadata& other) const
    {
        return !((*this) == other);
    }
};

std::ostream& operator<< (std::ostream& os, const Metadata& d);
}

#endif