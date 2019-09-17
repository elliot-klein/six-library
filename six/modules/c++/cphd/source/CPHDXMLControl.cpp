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

#include <io/StringStream.h>
#include <logging/NullLogger.h>
#include <six/Utilities.h>
#include <six/SICommonXMLParser.h>
#include <cphd/CPHDXMLControl.h>
#include <cphd/Metadata.h>

#include <mem/ScopedCopyablePtr.h>
#include <cphd/Enums.h>
#include <cphd/Types.h>

#include <set>

// CPHD Spec is not enforced
#define ENFORCESPEC 0 // TODO: Kill?

namespace
{
typedef xml::lite::Element* XMLElem;
}

namespace cphd
{
const char CPHDXMLControl::CPHD10_URI[] = "urn:CPHD:1.0.0";

CPHDXMLControl::CPHDXMLControl() :
    six::XMLParser(CPHD10_URI, false, new logging::NullLogger(), true),
    mCommon(CPHD10_URI, false, CPHD10_URI, log())
{
}

CPHDXMLControl::CPHDXMLControl(logging::Logger* log, bool ownLog) :
    six::XMLParser(CPHD10_URI, false, log, ownLog),
    mCommon(CPHD10_URI, false, CPHD10_URI, log)
{
}

CPHDXMLControl::CPHDXMLControl(logging::Logger* log, bool ownLog, std::vector<std::string>& schemaPaths) :
    six::XMLParser(CPHD10_URI, false, log, ownLog),
    mCommon(CPHD10_URI, false, CPHD10_URI, log),
    mSchemaPaths(schemaPaths)
{
}


std::string CPHDXMLControl::getDefaultURI() const
{
    return CPHD10_URI;
}

std::string CPHDXMLControl::getSICommonURI() const
{
    return CPHD10_URI;
}


void CPHDXMLControl::validate(const xml::lite::Document* doc,
              const std::vector<std::string>& schemaPaths,
              logging::Logger* log)
{
    // attempt to get the schema location from the
    // environment if nothing is specified
    std::vector<std::string> paths(schemaPaths);
    sys::OS os;
    try
    {
        if (paths.empty())
        {
            std::string envPath = os.getEnv(six::SCHEMA_PATH);
            str::trim(envPath);
            if (!envPath.empty())
            {
                paths.push_back(envPath);
            }
        }
    }
    catch (const except::Exception& )
    {
        // do nothing here
    }

    // validate against any specified schemas
    if (!paths.empty())
    {
        xml::lite::Validator validator(paths, log, true);

        std::vector<xml::lite::ValidationInfo> errors;

        if (doc->getRootElement()->getUri().empty())
        {
            throw six::DESValidationException(Ctxt(
                "INVALID XML: URI is empty so document version cannot be "
                "determined to use for validation"));
        }

        validator.validate(doc->getRootElement(),
                           doc->getRootElement()->getUri(),
                           errors);

        // log any error found and throw
        if (!errors.empty())
        {
            for (size_t i = 0; i < errors.size(); ++i)
            {
                log->critical(errors[i].toString());
            }

            //! this is a unique error thrown only in this location --
            //  if the user wants a file written regardless of the consequences
            //  they can catch this error, clear the vector and SIX_SCHEMA_PATH
            //  and attempt to rewrite the file. Continuing in this manner is
            //  highly discouraged
            for (size_t i = 0; i < errors.size(); ++i)
            {
                std::cout << errors[i].toString();
            }
            throw six::DESValidationException(Ctxt(
                "INVALID XML: Check both the XML being " \
                "produced and the schemas available"));
        }
    }
}

std::string CPHDXMLControl::toXMLString(const Metadata& metadata)
{
    std::auto_ptr<xml::lite::Document> doc(toXML(metadata));
    io::StringStream ss;
    doc->getRootElement()->prettyPrint(ss);

    return (std::string("<?xml version=\"1.0\"?>") + ss.stream().str());
}

/*
 * TO XML
 */
std::auto_ptr<xml::lite::Document> CPHDXMLControl::toXML(const Metadata& metadata)
{
    std::auto_ptr<xml::lite::Document> doc(new xml::lite::Document());

    XMLElem root = newElement("CPHD");
    doc->setRootElement(root);

    toXML(metadata.collectionID, root);
    toXML(metadata.global, root);
    toXML(metadata.sceneCoordinates, root);
    toXML(metadata.data, root);
    toXML(metadata.channel, root);
    toXML(metadata.pvp, root);
    if (metadata.supportArray.get())
    {
        toXML(*(metadata.supportArray), root);
    }
    toXML(metadata.dwell, root);
    toXML(metadata.referenceGeometry, root);
    if (metadata.antenna.get())
    {
        toXML(*(metadata.antenna), root);
    }
    if (metadata.txRcv.get())
    {
        toXML(*(metadata.txRcv), root);
    }
    if (metadata.errorParameters.get())
    {
        toXML(*(metadata.errorParameters), root);
    }
    if (metadata.productInfo.get())
    {
        toXML(*(metadata.productInfo), root);
    }
    for (size_t i = 0; i < metadata.geoInfo.size(); ++i)
    {
        toXML(metadata.geoInfo[i], root);
    }
    if (metadata.matchInfo.get())
    {
        toXML(*(metadata.matchInfo), root);
    }

    //set the XMLNS
    root->setNamespacePrefix("", getDefaultURI());

    return doc;
}

XMLElem CPHDXMLControl::toXML(const CollectionID& collectionID, XMLElem parent)
{
    XMLElem collectionXML = newElement("CollectionID", parent);

    createString("CollectorName", collectionID.collectorName, collectionXML);
    if(!six::Init::isUndefined(collectionID.illuminatorName))
    {
        createString("IlluminatorName", collectionID.illuminatorName, collectionXML);
    }
    createString("CoreName", collectionID.coreName, collectionXML);
    createString("CollectType", collectionID.collectType, collectionXML);

    // RadarMode
    XMLElem radarModeXML = newElement("RadarMode", collectionXML);
    createString("ModeType", collectionID.radarMode.toString(), radarModeXML);
    if(!six::Init::isUndefined(collectionID.radarModeID))
    {
        createString("ModeID", collectionID.radarModeID, radarModeXML);
    }

    createString("Classification", collectionID.getClassificationLevel(), collectionXML);
    createString("ReleaseInfo", collectionID.releaseInfo, collectionXML);
    std::string countryCodes = "";
    for (size_t i = 0; i < collectionID.countryCodes.size() - 1; ++i)
    {
        countryCodes += collectionID.countryCodes[i] + ",";
    }
    countryCodes += collectionID.countryCodes.back();
    createString("CountryCode", countryCodes, collectionXML);
    mCommon.addParameters("Parameter", getDefaultURI(), collectionID.parameters, collectionXML);
    return collectionXML;
}

XMLElem CPHDXMLControl::toXML(const Global& global, XMLElem parent)
{
    XMLElem globalXML = newElement("Global", parent);
    createString("DomainType", global.domainType, globalXML);
    createString("SGN", global.sgn.toString(), globalXML);

    //Timeline
    XMLElem timelineXML = newElement("Timeline", globalXML);
    createDateTime("CollectionStart", global.timeline.collectionStart, timelineXML);
    if (!six::Init::isUndefined(global.timeline.rcvCollectionStart))
    {
        createDateTime("RcvCollectionStart", global.timeline.rcvCollectionStart, timelineXML);
    }
    createDouble("TxTime1", global.timeline.txTime1, timelineXML);
    createDouble("TxTime2", global.timeline.txTime2, timelineXML);

    XMLElem fxBandXML = newElement("FxBand", globalXML);
    createDouble("FxMin", global.fxBand.fxMin, fxBandXML);
    createDouble("FxMax", global.fxBand.fxMax, fxBandXML);

    XMLElem toaSwathXML = newElement("TOASwath", globalXML);
    createDouble("TOAMin", global.toaSwath.toaMin, toaSwathXML);
    createDouble("TOAMax", global.toaSwath.toaMax, toaSwathXML);

    if (global.tropoParameters.get())
    {
        XMLElem tropoXML = newElement("TropoParameters", globalXML);
        createDouble("N0", global.tropoParameters->n0, tropoXML);
        createString("RefHeight", global.tropoParameters->refHeight, tropoXML);
    }
    if (global.ionoParameters.get())
    {
        XMLElem ionoXML = newElement("IonoParameters", globalXML);
        createDouble("TECV", global.ionoParameters->tecv, ionoXML);
        createDouble("F2Height", global.ionoParameters->f2Height, ionoXML);
    }
    return globalXML;
}

XMLElem CPHDXMLControl::toXML(const SceneCoordinates& sceneCoords, XMLElem parent)
{
    XMLElem sceneCoordsXML = newElement("SceneCoordinates", parent);
    createString("EarthModel", sceneCoords.earthModel, sceneCoordsXML);

    XMLElem iarpXML = newElement("IARP", sceneCoordsXML);
    mCommon.createVector3D("ECF", sceneCoords.iarp.ecf, iarpXML);
    mCommon.createLatLonAlt("LLH", sceneCoords.iarp.llh, iarpXML);

    XMLElem refSurfXML = newElement("ReferenceSurface", sceneCoordsXML);
    if (sceneCoords.referenceSurface.planar.get())
    {
        XMLElem planarXML = newElement("Planar", refSurfXML);
        mCommon.createVector3D("uIAX", sceneCoords.referenceSurface.planar->uIax, planarXML);
        mCommon.createVector3D("uIAY", sceneCoords.referenceSurface.planar->uIay, planarXML);
    }
    else if (sceneCoords.referenceSurface.hae.get())
    {
        XMLElem haeXML = newElement("HAE", refSurfXML);
        mCommon.createLatLon("uIAXLL", sceneCoords.referenceSurface.hae->uIax, haeXML);
        mCommon.createLatLon("uIAYLL", sceneCoords.referenceSurface.hae->uIay, haeXML);
    }
    else
    {
        throw except::Exception(Ctxt(
                                "Reference Surface must be one of two types"));
    }

    XMLElem imageAreaXML = newElement("ImageArea", sceneCoordsXML);
    createVector2D("X1Y1", sceneCoords.imageArea.x1y1, imageAreaXML);
    createVector2D("X2Y2", sceneCoords.imageArea.x2y2, imageAreaXML);

    if (!sceneCoords.imageArea.polygon.empty())
    {
        XMLElem polygonXML = newElement("Polygon", imageAreaXML);
        setAttribute(polygonXML, "size", six::toString(sceneCoords.imageArea.polygon.size()));
        for (size_t i = 0; i < sceneCoords.imageArea.polygon.size(); ++i)
        {
            XMLElem vertexXML = createVector2D("Vertex", sceneCoords.imageArea.polygon[i], polygonXML);
            setAttribute(vertexXML, "index", six::toString(i+1));
        }
    }
    createLatLonFootprint("ImageAreaCornerPoints", "IACP", sceneCoords.imageAreaCorners, sceneCoordsXML);
    if(sceneCoords.extendedArea.get())
    {
        XMLElem extendedAreaXML = newElement("ExtendedArea", sceneCoordsXML);
        createVector2D("X1Y1", sceneCoords.extendedArea->x1y1, extendedAreaXML);
        createVector2D("X2Y2", sceneCoords.extendedArea->x2y2, extendedAreaXML);

        if (!sceneCoords.extendedArea->polygon.empty())
        {
            XMLElem polygonXML = newElement("Polygon", sceneCoordsXML);
            setAttribute(polygonXML, "size", six::toString(sceneCoords.extendedArea->polygon.size()));
            for (size_t i = 0; i < sceneCoords.extendedArea->polygon.size(); ++i)
            {
                XMLElem vertexXML = createVector2D("Vertex", sceneCoords.extendedArea->polygon[i], polygonXML);
                setAttribute(vertexXML, "index", six::toString(i+1));
            }
        }
    }

    // ImageGrid
    XMLElem imageGridXML = newElement("ImageGrid", sceneCoordsXML);
    if(sceneCoords.imageGrid.get())
    {
        if(!six::Init::isUndefined(sceneCoords.imageGrid->identifier))
        {
            createString("Identifier", sceneCoords.imageGrid->identifier, imageGridXML);
        }
        XMLElem iarpLocationXML = newElement("IARPLocation", imageGridXML);
        createDouble("Line", sceneCoords.imageGrid->iarpLocation.line, iarpLocationXML);
        createDouble("Sample", sceneCoords.imageGrid->iarpLocation.sample, iarpLocationXML);

        XMLElem iaxExtentXML = newElement("IAXExtent", imageGridXML);
        createDouble("LineSpacing", sceneCoords.imageGrid->xExtent.lineSpacing, iaxExtentXML);
        createInt("FirstLine", sceneCoords.imageGrid->xExtent.firstLine, iaxExtentXML);
        createInt("NumLines", sceneCoords.imageGrid->xExtent.numLines, iaxExtentXML);

        XMLElem iayExtentXML = newElement("IAYExtent", imageGridXML);
        createDouble("SampleSpacing", sceneCoords.imageGrid->yExtent.sampleSpacing, iayExtentXML);
        createInt("FirstSample", sceneCoords.imageGrid->yExtent.firstSample, iayExtentXML);
        createInt("NumSamples", sceneCoords.imageGrid->yExtent.numSamples, iayExtentXML);

        XMLElem segmentListXML = newElement("SegmentList", imageGridXML);
        createInt("NumSegments", sceneCoords.imageGrid->segments.size(), segmentListXML);

        for (size_t i = 0; i < sceneCoords.imageGrid->segments.size(); ++i)
        {
            XMLElem segmentXML = newElement("Segment", segmentListXML);
            createString("Identifier", sceneCoords.imageGrid->segments[i].identifier, segmentXML);
            createInt("StartLine", sceneCoords.imageGrid->segments[i].startLine, segmentXML);
            createInt("StartSample", sceneCoords.imageGrid->segments[i].startSample, segmentXML);
            createInt("EndLine", sceneCoords.imageGrid->segments[i].endLine, segmentXML);
            createInt("EndSample", sceneCoords.imageGrid->segments[i].endSample, segmentXML);

            XMLElem polygonXML = newElement("SegmentPolygon", segmentXML);
            setAttribute(polygonXML, "size", six::toString(sceneCoords.imageGrid->segments[i].size));
            for (size_t j = 0; j < sceneCoords.imageGrid->segments[i].polygon.size(); ++j)
            {
                XMLElem svXML = newElement("SV", polygonXML);
                setAttribute(svXML, "index", six::toString(sceneCoords.imageGrid->segments[i].polygon[j].getIndex()));
                createDouble("Line", sceneCoords.imageGrid->segments[i].polygon[j].line, svXML);
                createDouble("Sample", sceneCoords.imageGrid->segments[i].polygon[j].sample, svXML);
            }
        }
    }
    return sceneCoordsXML;
}

XMLElem CPHDXMLControl::toXML(const Data& data, XMLElem parent)
{
    XMLElem dataXML = newElement("Data", parent);
    createString("SignalArrayFormat", data.signalArrayFormat, dataXML);
    createInt("NumBytesPVP", data.numBytesPVP, dataXML);
    createInt("NumCPHDChannels", data.channels.size(), dataXML);
    if (!six::Init::isUndefined(data.signalCompressionID))
    {
        createString("SignalCompressionID", data.signalCompressionID, dataXML);
    }

    for (size_t i = 0; i < data.channels.size(); ++i)
    {
        XMLElem channelXML = newElement("Channel", dataXML);
        createString("Identifier", data.channels[i].identifier, channelXML);
        createInt("NumVectors", data.channels[i].numVectors, channelXML);
        createInt("NumSamples", data.channels[i].numSamples, channelXML);
        createInt("SignalArrayByteOffset", data.channels[i].signalArrayByteOffset, channelXML);
        createInt("PVPArrayByteOffset", data.channels[i].pvpArrayByteOffset, channelXML);
        if(!six::Init::isUndefined(data.channels[i].compressedSignalSize))
        {
            createInt("CompressedSignalSize", data.channels[i].compressedSignalSize, channelXML);
        }
    }
    createInt("NumSupportArrays", data.supportArrays.size(), dataXML);
    for (size_t i = 0; i < data.supportArrays.size(); ++i)
    {
        XMLElem supportArrayXML = newElement("SupportArray", dataXML);
        createString("Identifier", data.supportArrays[i].identifier, supportArrayXML);
        createInt("NumRows", data.supportArrays[i].numRows, supportArrayXML);
        createInt("NumCols", data.supportArrays[i].numCols, supportArrayXML);
        createInt("BytesPerElement", data.supportArrays[i].bytesPerElement, supportArrayXML);
        createInt("ArrayByteOffset", data.supportArrays[i].arrayByteOffset, supportArrayXML);
    }
    return dataXML;
}

XMLElem CPHDXMLControl::toXML(const Channel& channel, XMLElem parent)
{
    XMLElem channelXML = newElement("Channel", parent);
    createString("RefChId", channel.refChId, channelXML);
    createBooleanType("FXFixedCPHD", channel.fxFixedCphd, channelXML);
    createBooleanType("TOAFixedCPHD", channel.toaFixedCphd, channelXML);
    createBooleanType("SRPFixedCPHD", channel.srpFixedCphd, channelXML);

    for (size_t i = 0; i < channel.parameters.size(); ++i)
    {
        XMLElem parametersXML = newElement("Parameters", channelXML);
        createString("Identifier", channel.parameters[i].identifier, parametersXML);
        createInt("RefVectorIndex", channel.parameters[i].refVectorIndex, parametersXML);
        createBooleanType("FXFixed", channel.parameters[i].fxFixed, parametersXML);
        createBooleanType("TOAFixed", channel.parameters[i].toaFixed, parametersXML);
        createBooleanType("SRPFixed", channel.parameters[i].srpFixed, parametersXML);
        if (!six::Init::isUndefined(channel.parameters[i].signalNormal))
        {
            createBooleanType("SignalNormal", channel.parameters[i].signalNormal, parametersXML);
        }
        XMLElem polXML = newElement("Polarization", parametersXML);
        createString("TxPol", channel.parameters[i].polarization.txPol.toString(), polXML);
        createString("RcvPol", channel.parameters[i].polarization.rcvPol.toString(), polXML);
        createDouble("FxC", channel.parameters[i].fxC, parametersXML);
        createDouble("FxBW", channel.parameters[i].fxBW, parametersXML);
        if(!six::Init::isUndefined(channel.parameters[i].fxBWNoise))
        {
            createDouble("FxBWNoise", channel.parameters[i].fxBWNoise, parametersXML);
        }
        createDouble("TOASaved", channel.parameters[i].toaSaved, parametersXML);

        if(channel.parameters[i].toaExtended.get())
        {
            XMLElem toaExtendedXML = newElement("TOAExtended", parametersXML);
            createDouble("TOAExtSaved", channel.parameters[i].toaExtended->toaExtSaved, toaExtendedXML);
            if(channel.parameters[i].toaExtended->lfmEclipse.get())
            {
                XMLElem lfmEclipseXML = newElement("LFMEclipse", toaExtendedXML);
                createDouble("FxEarlyLow", channel.parameters[i].toaExtended->lfmEclipse->fxEarlyLow, lfmEclipseXML);
                createDouble("FxEarlyHigh", channel.parameters[i].toaExtended->lfmEclipse->fxEarlyHigh, lfmEclipseXML);
                createDouble("FxLateLow", channel.parameters[i].toaExtended->lfmEclipse->fxLateLow, lfmEclipseXML);
                createDouble("FxLateHigh", channel.parameters[i].toaExtended->lfmEclipse->fxLateHigh, lfmEclipseXML);
            }
        }
        XMLElem dwellTimesXML = newElement("DwellTimes", parametersXML);
        createString("CODId", channel.parameters[i].dwellTimes.codId, dwellTimesXML);
        createString("DwellId", channel.parameters[i].dwellTimes.dwellId, dwellTimesXML);
        if(!six::Init::isUndefined(channel.parameters[i].imageArea))
        {
            XMLElem imageAreaXML = newElement("ImageArea", parametersXML);
            createVector2D("X1Y1", channel.parameters[i].imageArea.x1y1, imageAreaXML);
            createVector2D("X2Y2", channel.parameters[i].imageArea.x2y2, imageAreaXML);
            if(!channel.parameters[i].imageArea.polygon.empty())
            {
                XMLElem polygonXML = newElement("Polygon", imageAreaXML);
                setAttribute(polygonXML, "size", six::toString(channel.parameters[i].imageArea.polygon.size()));
                for (size_t j = 0; j < channel.parameters[i].imageArea.polygon.size(); ++j)
                {
                    XMLElem vertexXML = createVector2D("Vertex", channel.parameters[i].imageArea.polygon[j], polygonXML);
                    setAttribute(vertexXML, "index", six::toString(j+1));
                }
            }
        }
        if(channel.parameters[i].antenna.get())
        {
            XMLElem antennaXML = newElement("Antenna", parametersXML);
            createString("TxAPCId", channel.parameters[i].antenna->txAPCId, antennaXML);
            createString("TxAPATId", channel.parameters[i].antenna->txAPATId, antennaXML);
            createString("RcvAPCId", channel.parameters[i].antenna->rcvAPCId, antennaXML);
            createString("RcvAPATId", channel.parameters[i].antenna->rcvAPATId, antennaXML);
        }
        if(channel.parameters[i].txRcv.get())
        {
            XMLElem txRcvXML = newElement("TxRcv", parametersXML);
            for (size_t j = 0; j < channel.parameters[i].txRcv->txWFId.size(); ++j)
            {
                createString("TxWFId", channel.parameters[i].txRcv->txWFId[j], txRcvXML);
            }
            for (size_t j = 0; j < channel.parameters[i].txRcv->rcvId.size(); ++j)
            {
                createString("RcvId", channel.parameters[i].txRcv->rcvId[j], txRcvXML);
            }
        }
        if(channel.parameters[i].tgtRefLevel.get())
        {
            XMLElem tgtRefXML = newElement("TgtRefLevel", parametersXML);
            createDouble("PTRef", channel.parameters[i].tgtRefLevel->ptRef, tgtRefXML);
        }
        if(channel.parameters[i].noiseLevel.get())
        {
            XMLElem noiseLevelXML = newElement("NoiseLevel", parametersXML);
            createDouble("PNRef", channel.parameters[i].noiseLevel->pnRef, noiseLevelXML);
            createDouble("BNRef", channel.parameters[i].noiseLevel->bnRef, noiseLevelXML);
            if(channel.parameters[i].noiseLevel->fxNoiseProfile.get())
            {
                XMLElem fxNoiseProfileXML = newElement("FxNoiseProfile", noiseLevelXML);
                for (size_t j = 0; j < channel.parameters[i].noiseLevel->fxNoiseProfile->point.size(); ++j)
                {
                    XMLElem pointXML = newElement("Point", fxNoiseProfileXML);
                    createDouble("Fx", channel.parameters[i].noiseLevel->fxNoiseProfile->point[j].fx, pointXML);
                    createDouble("PN", channel.parameters[i].noiseLevel->fxNoiseProfile->point[j].pn, pointXML);
                }
            }
        }
    }
    if(channel.addedParameters.size() > 0)
    {
        XMLElem addedParamsXML = newElement("AddedParameters", channelXML);
        mCommon.addParameters("Parameter", getDefaultURI(), channel.addedParameters, addedParamsXML);
    }
    return channelXML;
}

XMLElem CPHDXMLControl::toXML(const Pvp& pvp, XMLElem parent)
{
    XMLElem pvpXML = newElement("PVP", parent);
    createPVPType("TxTime", pvp.txTime, pvpXML);
    createPVPType("TxPos", pvp.txPos, pvpXML);
    createPVPType("TxVel", pvp.txVel, pvpXML);
    createPVPType("RcvTime", pvp.rcvTime, pvpXML);
    createPVPType("RcvPos", pvp.rcvPos, pvpXML);
    createPVPType("RcvVel", pvp.rcvVel, pvpXML);
    createPVPType("SRPPos", pvp.srpPos, pvpXML);
    if (pvp.ampSF.get())
    {
        createPVPType("AmpSF", *pvp.ampSF, pvpXML);
    }
    createPVPType("aFDOP", pvp.aFDOP, pvpXML);
    createPVPType("aFRR1", pvp.aFRR1, pvpXML);
    createPVPType("aFRR2", pvp.aFRR2, pvpXML);
    createPVPType("FX1", pvp.fx1, pvpXML);
    createPVPType("FX2", pvp.fx2, pvpXML);
    if (pvp.fxN1.get())
    {
        createPVPType("FxN1", *pvp.fxN1, pvpXML);
    }
    if (pvp.fxN2.get())
    {
        createPVPType("FxN2", *pvp.fxN2, pvpXML);
    }
    createPVPType("TOA1", pvp.toa1, pvpXML);
    createPVPType("TOA2", pvp.toa2, pvpXML);
    if (pvp.toaE1.get())
    {
        createPVPType("TOAE1", *pvp.toaE1, pvpXML);
    }
    if (pvp.toaE2.get())
    {
        createPVPType("TOAE2", *pvp.toaE2, pvpXML);
    }
    createPVPType("TDTropoSRP", pvp.tdTropoSRP, pvpXML);
    if (pvp.tdIonoSRP.get())
    {
        createPVPType("TDIonoSRP", *pvp.tdIonoSRP, pvpXML);
    }
    createPVPType("SC0", pvp.sc0, pvpXML);
    createPVPType("SCSS", pvp.scss, pvpXML);
    for (size_t i = 0; i < pvp.addedPVP.size(); ++i)
    {
        createAPVPType("AddedPVP", pvp.addedPVP[i], pvpXML);
    }

    return pvpXML;
}


//Assumes optional handled by caller
XMLElem CPHDXMLControl::toXML(const SupportArray& supports, XMLElem parent)
{
    XMLElem supportsXML = newElement("SupportArray", parent);
    if (!supports.iazArray.empty())
    {
        for (size_t i = 0; i < supports.iazArray.size(); ++i)
        {
            XMLElem iazArrayXML = newElement("IAZArray", supportsXML);
            createInt("Identifier", supports.iazArray[i].getIdentifier(), iazArrayXML);
            createString("ElementFormat", supports.iazArray[i].elementFormat, iazArrayXML);
            createDouble("X0", supports.iazArray[i].x0, iazArrayXML);
            createDouble("Y0", supports.iazArray[i].y0, iazArrayXML);
            createDouble("XSS", supports.iazArray[i].xSS, iazArrayXML);
            createDouble("YSS", supports.iazArray[i].ySS, iazArrayXML);
        }
    }
    if (!supports.antGainPhase.empty())
    {
        for (size_t i = 0; i < supports.antGainPhase.size(); ++i)
        {
            XMLElem antGainPhaseXML = newElement("AntGainPhase", supportsXML);
            createInt("Identifier", supports.antGainPhase[i].getIdentifier(), antGainPhaseXML);
            createString("ElementFormat", supports.antGainPhase[i].elementFormat, antGainPhaseXML);
            createDouble("X0", supports.antGainPhase[i].x0, antGainPhaseXML);
            createDouble("Y0", supports.antGainPhase[i].y0, antGainPhaseXML);
            createDouble("XSS", supports.antGainPhase[i].xSS, antGainPhaseXML);
            createDouble("YSS", supports.antGainPhase[i].ySS, antGainPhaseXML);
        }
    }
    if (!supports.addedSupportArray.empty())
    {
        for (size_t i = 0; i < supports.addedSupportArray.size(); ++i)
        {
            XMLElem addedSupportArrayXML = newElement("AddedSupportArray", supportsXML);
            createString("Identifier", supports.addedSupportArray[i].identifier, addedSupportArrayXML);
            createString("ElementFormat", supports.addedSupportArray[i].elementFormat, addedSupportArrayXML);
            createDouble("X0", supports.addedSupportArray[i].x0, addedSupportArrayXML);
            createDouble("Y0", supports.addedSupportArray[i].y0, addedSupportArrayXML);
            createDouble("XSS", supports.addedSupportArray[i].xSS, addedSupportArrayXML);
            createDouble("YSS", supports.addedSupportArray[i].ySS, addedSupportArrayXML);
            createString("XUnits", supports.addedSupportArray[i].xUnits, addedSupportArrayXML);
            createString("YUnits", supports.addedSupportArray[i].yUnits, addedSupportArrayXML);
            createString("ZUnits", supports.addedSupportArray[i].zUnits, addedSupportArrayXML);
            mCommon.addParameters("Parameter", getDefaultURI(), supports.addedSupportArray[i].parameter, addedSupportArrayXML);
        }
    }
    return supportsXML;
}

XMLElem CPHDXMLControl::toXML(const Dwell& dwell, XMLElem parent)
{
    XMLElem dwellXML = newElement("Dwell", parent);
    createInt("NumCODTimes", dwell.numCODTimes, dwellXML);

    for (size_t i = 0; i < dwell.cod.size(); ++i)
    {
        XMLElem codTimeXML = newElement("CODTime", dwellXML);
        createString("Identifier", dwell.cod[i].identifier, codTimeXML);
        mCommon.createPoly2D("CODTimePoly", dwell.cod[i].codTimePoly, codTimeXML);
    }
    createInt("NumDwellTimes", dwell.numDwellTimes, dwellXML);
    for (size_t i = 0; i < dwell.dtime.size(); ++i)
    {
        XMLElem dwellTimeXML = newElement("DwellTime", dwellXML);
        createString("Identifier", dwell.dtime[i].identifier, dwellTimeXML);
        mCommon.createPoly2D("DwellTimePoly", dwell.dtime[i].dwellTimePoly, dwellTimeXML);
    }
    return dwellXML;
}

XMLElem CPHDXMLControl::toXML(const ReferenceGeometry& refGeo, XMLElem parent)
{
    XMLElem refGeoXML = newElement("ReferenceGeometry", parent);
    XMLElem srpXML = newElement("SRP", refGeoXML);
    mCommon.createVector3D("ECF", refGeo.srp.ecf, srpXML);
    mCommon.createVector3D("IAC", refGeo.srp.iac, srpXML);
    createDouble("ReferenceTime", refGeo.referenceTime, refGeoXML);
    createDouble("SRPCODTime", refGeo.srpCODTime, refGeoXML);
    createDouble("SRPDwellTime", refGeo.srpDwellTime, refGeoXML);
    
    if (refGeo.monostatic.get())
    {
        XMLElem monoXML = newElement("Monostatic", refGeoXML);
        mCommon.createVector3D("ARPPos", refGeo.monostatic->arpPos, monoXML);
        mCommon.createVector3D("ARPVel", refGeo.monostatic->arpVel, monoXML);
        std::string side = refGeo.monostatic->sideOfTrack.toString();
        createString("SideOfTrack", (side == "LEFT" ? "L" : "R"), monoXML);
        createDouble("SlantRange", refGeo.monostatic->slantRange, monoXML);
        createDouble("GroundRange", refGeo.monostatic->groundRange, monoXML);
        createDouble("DopplerConeAngle", refGeo.monostatic->dopplerConeAngle, monoXML);
        createDouble("GrazeAngle", refGeo.monostatic->grazeAngle, monoXML);
        createDouble("IncidenceAngle", refGeo.monostatic->incidenceAngle, monoXML);
        createDouble("AzimuthAngle", refGeo.monostatic->azimuthAngle, monoXML);
        createDouble("TwistAngle", refGeo.monostatic->twistAngle, monoXML);
        createDouble("SlopeAngle", refGeo.monostatic->slopeAngle, monoXML);
        createDouble("LayoverAngle", refGeo.monostatic->layoverAngle, monoXML);
    }
    else if(refGeo.bistatic.get())
    {
        XMLElem biXML = newElement("Bistatic", refGeoXML);
        createDouble("AzimuthAngle", refGeo.bistatic->azimuthAngle, biXML);
        createDouble("AzimuthAngleRate", refGeo.bistatic->azimuthAngleRate, biXML);
        createDouble("BistaticAngle", refGeo.bistatic->bistaticAngle, biXML);
        createDouble("BistaticAngleRate", refGeo.bistatic->bistaticAngleRate, biXML);
        createDouble("GrazeAngle", refGeo.bistatic->grazeAngle, biXML);
        createDouble("TwistAngle", refGeo.bistatic->twistAngle, biXML);
        createDouble("SlopeAngle", refGeo.bistatic->slopeAngle, biXML);
        createDouble("LayoverAngle", refGeo.bistatic->layoverAngle, biXML);
        XMLElem txPlatXML = newElement("TxPlatform", biXML);
        createDouble("Time", refGeo.bistatic->txPlatform.time, txPlatXML);
        mCommon.createVector3D("Pos", refGeo.bistatic->txPlatform.pos, txPlatXML);
        mCommon.createVector3D("Vel", refGeo.bistatic->txPlatform.vel, txPlatXML);

        std::string side = refGeo.bistatic->txPlatform.sideOfTrack.toString();
        createString("SideOfTrack", (side == "LEFT" ? "L" : "R"), txPlatXML);
        createDouble("SlantRange", refGeo.bistatic->txPlatform.slantRange, txPlatXML);
        createDouble("GroundRange", refGeo.bistatic->txPlatform.groundRange, txPlatXML);
        createDouble("DopplerConeAngle", refGeo.bistatic->txPlatform.dopplerConeAngle, txPlatXML);
        createDouble("GrazeAngle", refGeo.bistatic->txPlatform.grazeAngle, txPlatXML);
        createDouble("IncidenceAngle", refGeo.bistatic->txPlatform.incidenceAngle, txPlatXML);
        createDouble("AzimuthAngle", refGeo.bistatic->txPlatform.azimuthAngle, txPlatXML);
        XMLElem rcvPlatXML = newElement("RcvPlatform", biXML);
        createDouble("Time", refGeo.bistatic->rcvPlatform.time, rcvPlatXML);
        mCommon.createVector3D("Pos", refGeo.bistatic->rcvPlatform.pos, rcvPlatXML);
        mCommon.createVector3D("Vel", refGeo.bistatic->rcvPlatform.vel, rcvPlatXML);

        side = refGeo.bistatic->rcvPlatform.sideOfTrack.toString();
        createString("SideOfTrack", (side == "LEFT" ? "L" : "R"), rcvPlatXML);
        createDouble("SlantRange", refGeo.bistatic->rcvPlatform.slantRange, rcvPlatXML);
        createDouble("GroundRange", refGeo.bistatic->rcvPlatform.groundRange, rcvPlatXML);
        createDouble("DopplerConeAngle", refGeo.bistatic->rcvPlatform.dopplerConeAngle, rcvPlatXML);
        createDouble("GrazeAngle", refGeo.bistatic->rcvPlatform.grazeAngle, rcvPlatXML);
        createDouble("IncidenceAngle", refGeo.bistatic->rcvPlatform.incidenceAngle, rcvPlatXML);
        createDouble("AzimuthAngle", refGeo.bistatic->rcvPlatform.azimuthAngle, rcvPlatXML);
    }
    return refGeoXML;
}

XMLElem CPHDXMLControl::toXML(const Antenna& antenna, XMLElem parent)
{
    XMLElem antennaXML = newElement("Antenna", parent);
    createInt("NumACFs", antenna.numACFs, antennaXML);
    createInt("NumAPCs", antenna.numAPCs, antennaXML);
    createInt("NumAntPats", antenna.numAntPats, antennaXML);
    for (size_t i = 0; i < antenna.antCoordFrame.size(); ++i)
    {
        XMLElem antCoordFrameXML = newElement("AntCoordFrame", antennaXML);
        createString("Identifier", antenna.antCoordFrame[i].identifier, antCoordFrameXML);
        mCommon.createPolyXYZ("XAxisPoly", antenna.antCoordFrame[i].xAxisPoly, antCoordFrameXML);
        mCommon.createPolyXYZ("YAxisPoly", antenna.antCoordFrame[i].yAxisPoly, antCoordFrameXML);
    }
    for (size_t i = 0; i < antenna.antPhaseCenter.size(); ++i)
    {
        XMLElem antPhaseCenterXML = newElement("AntPhaseCenter", antennaXML);
        createString("Identifier", antenna.antPhaseCenter[i].identifier, antPhaseCenterXML);
        createString("ACFId", antenna.antPhaseCenter[i].acfId, antPhaseCenterXML);
        mCommon.createVector3D("APCXYZ", antenna.antPhaseCenter[i].apcXYZ, antPhaseCenterXML);
    }
    for (size_t i = 0; i < antenna.antPattern.size(); ++i)
    {
        XMLElem antPatternXML = newElement("AntPattern", antennaXML);
        createString("Identifier", antenna.antPattern[i].identifier, antPatternXML);
        createDouble("FreqZero", antenna.antPattern[i].freqZero, antPatternXML);
        createDouble("GainZero", antenna.antPattern[i].gainZero, antPatternXML);
        createBooleanType("EBFreqShift", antenna.antPattern[i].ebFreqShift, antPatternXML);
        createBooleanType("MLFreqDilation", antenna.antPattern[i].mlFreqDilation, antPatternXML);
        mCommon.createPoly1D("GainBSPoly", antenna.antPattern[i].gainBSPoly, antPatternXML);
        XMLElem ebXML = newElement("EB", antPatternXML);
        mCommon.createPoly1D("DCXPoly", antenna.antPattern[i].eb.dcXPoly, ebXML);
        mCommon.createPoly1D("DCYPoly", antenna.antPattern[i].eb.dcYPoly, ebXML);
        XMLElem arrayXML = newElement("Array", antPatternXML);
        mCommon.createPoly2D("GainPoly", antenna.antPattern[i].array.gainPoly, arrayXML);
        mCommon.createPoly2D("PhasePoly", antenna.antPattern[i].array.phasePoly, arrayXML);
        XMLElem elementXML = newElement("Element", antPatternXML);
        mCommon.createPoly2D("GainPoly", antenna.antPattern[i].element.gainPoly, elementXML);
        mCommon.createPoly2D("PhasePoly", antenna.antPattern[i].element.phasePoly, elementXML);
        for (size_t j = 0; j < antenna.antPattern[i].gainPhaseArray.size(); ++j)
        {
            XMLElem gainPhaseArrayXML = newElement("GainPhaseArray", antPatternXML);
            createDouble("Freq", antenna.antPattern[i].gainPhaseArray[j].freq, gainPhaseArrayXML);
            createString("ArrayId", antenna.antPattern[i].gainPhaseArray[j].arrayId, gainPhaseArrayXML);
            createString("ElementId", antenna.antPattern[i].gainPhaseArray[j].elementId, gainPhaseArrayXML);
        }
    }
    return antennaXML;
}

XMLElem CPHDXMLControl::toXML(const TxRcv& txRcv, XMLElem parent)
{
    XMLElem txRcvXML = newElement("TxRcv", parent);
    createInt("NumTxWFs", txRcv.numTxWFs, txRcvXML);
    for (size_t i = 0; i < txRcv.txWFParameters.size(); ++i)
    {
        XMLElem txWFParamsXML = newElement("TxWFParameters", txRcvXML);
        createString("Identifier", txRcv.txWFParameters[i].identifier, txWFParamsXML);
        createDouble("PulseLength", txRcv.txWFParameters[i].pulseLength, txWFParamsXML);
        createDouble("RFBandwidth", txRcv.txWFParameters[i].rfBandwidth, txWFParamsXML);
        createDouble("FreqCenter", txRcv.txWFParameters[i].freqCenter, txWFParamsXML);
        createDouble("LFMRate", txRcv.txWFParameters[i].lfmRate, txWFParamsXML);
        createString("Polarization", txRcv.txWFParameters[i].polarization, txWFParamsXML);
        createDouble("Power", txRcv.txWFParameters[i].power, txWFParamsXML);
    }
    createInt("NumRcvs", txRcv.numRcvs, txRcvXML);
    for (size_t i = 0; i < txRcv.rcvParameters.size(); ++i)
    {
        XMLElem rcvParamsXML = newElement("RcvParameters", txRcvXML);
        createString("Identifier", txRcv.rcvParameters[i].identifier, rcvParamsXML);
        createDouble("WindowLength", txRcv.rcvParameters[i].windowLength, rcvParamsXML);
        createDouble("SampleRate", txRcv.rcvParameters[i].sampleRate, rcvParamsXML);
        createDouble("IFFilterBW", txRcv.rcvParameters[i].ifFilterBW, rcvParamsXML);
        createDouble("FreqCenter", txRcv.rcvParameters[i].freqCenter, rcvParamsXML);
        createDouble("LFMRate", txRcv.rcvParameters[i].lfmRate, rcvParamsXML);
        createString("Polarization", txRcv.rcvParameters[i].polarization, rcvParamsXML);
        createDouble("PathGain", txRcv.rcvParameters[i].pathGain, rcvParamsXML);
    }
    return txRcvXML;
}

XMLElem CPHDXMLControl::toXML(const ErrorParameters& errParams, XMLElem parent)
{
    XMLElem errParamsXML = newElement("ErrorParameters", parent);
    if (errParams.monostatic.get())
    {
        XMLElem monoXML = newElement("Monostatic", errParamsXML);
        XMLElem posVelErrXML = newElement("PosVelErr", monoXML);
        createString("Frame", errParams.monostatic->posVelErr.frame.toString(), posVelErrXML);
        createDouble("P1", errParams.monostatic->posVelErr.p1, posVelErrXML);
        createDouble("P2", errParams.monostatic->posVelErr.p2, posVelErrXML);
        createDouble("P3", errParams.monostatic->posVelErr.p3, posVelErrXML);
        createDouble("V1", errParams.monostatic->posVelErr.v1, posVelErrXML);
        createDouble("V2", errParams.monostatic->posVelErr.v2, posVelErrXML);
        createDouble("V3", errParams.monostatic->posVelErr.v3, posVelErrXML);
        XMLElem corrCoefsXML = newElement("CorrCoefs", posVelErrXML);
        if(errParams.monostatic->posVelErr.corrCoefs.get())
        {
            createDouble("P1P2", errParams.monostatic->posVelErr.corrCoefs->p1p2, corrCoefsXML);
            createDouble("P1P3", errParams.monostatic->posVelErr.corrCoefs->p1p3, corrCoefsXML);
            createDouble("P1V1", errParams.monostatic->posVelErr.corrCoefs->p1v1, corrCoefsXML);
            createDouble("P1V2", errParams.monostatic->posVelErr.corrCoefs->p1v2, corrCoefsXML);
            createDouble("P1V3", errParams.monostatic->posVelErr.corrCoefs->p1v3, corrCoefsXML);
            createDouble("P2P3", errParams.monostatic->posVelErr.corrCoefs->p2p3, corrCoefsXML);
            createDouble("P2V1", errParams.monostatic->posVelErr.corrCoefs->p2v1, corrCoefsXML);
            createDouble("P2V2", errParams.monostatic->posVelErr.corrCoefs->p2v2, corrCoefsXML);
            createDouble("P2V3", errParams.monostatic->posVelErr.corrCoefs->p2v3, corrCoefsXML);
            createDouble("P3V1", errParams.monostatic->posVelErr.corrCoefs->p3v1, corrCoefsXML);
            createDouble("P3V2", errParams.monostatic->posVelErr.corrCoefs->p3v2, corrCoefsXML);
            createDouble("P3V3", errParams.monostatic->posVelErr.corrCoefs->p3v3, corrCoefsXML);
            createDouble("V1V2", errParams.monostatic->posVelErr.corrCoefs->v1v2, corrCoefsXML);
            createDouble("V1V3", errParams.monostatic->posVelErr.corrCoefs->v1v3, corrCoefsXML);
            createDouble("V2V3", errParams.monostatic->posVelErr.corrCoefs->v2v3, corrCoefsXML);
        }
        if(errParams.monostatic->posVelErr.positionDecorr.get())
        {
            XMLElem positionDecorrXML = newElement("PositionDecorr", posVelErrXML);
            createDouble("CorrCoefZero", errParams.monostatic->posVelErr.positionDecorr->corrCoefZero, positionDecorrXML);
            createDouble("DecorrRate", errParams.monostatic->posVelErr.positionDecorr->decorrRate, positionDecorrXML);
        }
        // RadarSensor
        XMLElem radarXML = newElement("RadarSensor", monoXML);
        createDouble("RangeBias", errParams.monostatic->radarSensor.rangeBias, radarXML);
        if (!six::Init::isUndefined(errParams.monostatic->radarSensor.clockFreqSF))
        {
            createDouble("ClockFreqSF", errParams.monostatic->radarSensor.clockFreqSF, radarXML);
        }
        if (!six::Init::isUndefined(errParams.monostatic->radarSensor.collectionStartTime))
        {
            createDouble("CollectionStartTime", errParams.monostatic->radarSensor.collectionStartTime, radarXML);
        }
        if (errParams.monostatic->radarSensor.rangeBiasDecorr.get())
        {
            XMLElem rangeBiasDecorrXML = newElement("RangeBiasDecorr", radarXML);
            createDouble("CorrCoefZero", errParams.monostatic->radarSensor.rangeBiasDecorr->corrCoefZero, rangeBiasDecorrXML);
            createDouble("DecorrRate", errParams.monostatic->radarSensor.rangeBiasDecorr->decorrRate, rangeBiasDecorrXML);
        }

        if (errParams.monostatic->tropoError.get())
        {
            XMLElem tropoXML = newElement("TropoError", monoXML);
            if (!six::Init::isUndefined(errParams.monostatic->tropoError->tropoRangeVertical))
            {
                createDouble("TropoRangeVertical", errParams.monostatic->tropoError->tropoRangeVertical, tropoXML);
            }
            if (!six::Init::isUndefined(errParams.monostatic->tropoError->tropoRangeSlant))
            {
                createDouble("TropoRangeSlant", errParams.monostatic->tropoError->tropoRangeSlant, tropoXML);
            }
            if (errParams.monostatic->tropoError->tropoRangeDecorr.get())
            {
                XMLElem tropoDecorrXML = newElement("TropoRangeDecorr", tropoXML);
                createDouble("CorrCoefZero", errParams.monostatic->tropoError->tropoRangeDecorr->corrCoefZero, tropoDecorrXML);
                createDouble("DecorrRate", errParams.monostatic->tropoError->tropoRangeDecorr->decorrRate, tropoDecorrXML);
            }
        }
        if (errParams.monostatic->ionoError.get())
        {
            XMLElem ionoXML = newElement("IonoError", monoXML);
            createDouble("IonoRangeVertical", errParams.monostatic->ionoError->ionoRangeVertical, ionoXML);
            if (!six::Init::isUndefined(errParams.monostatic->ionoError->ionoRangeRateVertical))
            {
                createDouble("IonoRangeRateVertical", errParams.monostatic->ionoError->ionoRangeRateVertical, ionoXML);
            }
            if (!six::Init::isUndefined(errParams.monostatic->ionoError->ionoRgRgRateCC))
            {
                createDouble("IonoRgRgRateCC", errParams.monostatic->ionoError->ionoRgRgRateCC, ionoXML);
            }
            if (errParams.monostatic->ionoError->ionoRangeVertDecorr.get())
            {
                XMLElem ionoDecorrXML = newElement("IonoRangeVertDecorr", ionoXML);
                createDouble("CorrCoefZero", errParams.monostatic->ionoError->ionoRangeVertDecorr->corrCoefZero, ionoDecorrXML);
                createDouble("DecorrRate", errParams.monostatic->ionoError->ionoRangeVertDecorr->decorrRate, ionoDecorrXML);
            }
        }
        if (errParams.monostatic->parameter.size() > 0)
        {
            XMLElem addedParamsXML = newElement("AddedParameters", monoXML);
            mCommon.addParameters("Parameter", getDefaultURI(), errParams.monostatic->parameter, addedParamsXML);
        }
    }
    else if (errParams.bistatic.get())
    {
        XMLElem biXML = newElement("Bistatic", errParamsXML);
        XMLElem txPlatXML = newElement("TxPlatform", biXML);
        createErrorParamPlatform("TxPlatform", errParams.bistatic->txPlatform, txPlatXML);
        XMLElem radarTxXML = newElement("RadarSensor", txPlatXML);
        if(!six::Init::isUndefined(errParams.bistatic->txPlatform.radarSensor.clockFreqSF))
        {
            createDouble("ClockFreqSF", errParams.bistatic->txPlatform.radarSensor.clockFreqSF, radarTxXML);
        }
        createDouble("CollectionStartTime", errParams.bistatic->txPlatform.radarSensor.collectionStartTime, radarTxXML);

        XMLElem rcvPlatXML = newElement("RcvPlatform", biXML);
        createErrorParamPlatform("RcvPlatform", errParams.bistatic->rcvPlatform, rcvPlatXML);
        XMLElem radarRcvXML = newElement("RadarSensor", rcvPlatXML);
        if(!six::Init::isUndefined(errParams.bistatic->rcvPlatform.radarSensor.clockFreqSF))
        {
            createDouble("ClockFreqSF", errParams.bistatic->rcvPlatform.radarSensor.clockFreqSF, radarRcvXML);
        }
        createDouble("CollectionStartTime", errParams.bistatic->rcvPlatform.radarSensor.collectionStartTime, radarRcvXML);

        if (errParams.bistatic->parameter.size() > 0)
        {
            XMLElem addedParamsXML = newElement("AddedParameters", biXML);
            mCommon.addParameters("Parameter",  getDefaultURI(), errParams.bistatic->parameter, addedParamsXML);
        }
    }

    return errParamsXML;
}

XMLElem CPHDXMLControl::toXML(const ProductInfo& productInfo, XMLElem parent)
{
    XMLElem productInfoXML = newElement("ProductInfo", parent);
    if(!six::Init::isUndefined(productInfo.profile))
    {
        createString("Profile", productInfo.profile, productInfoXML);
    }
    for (size_t i = 0; i < productInfo.creationInfo.size(); ++i)
    {
        XMLElem creationInfoXML = newElement("CreationInfo", productInfoXML);
        if(!six::Init::isUndefined(productInfo.creationInfo[i].application))
        {
            createString("Application", productInfo.creationInfo[i].application, creationInfoXML);
        }
        createDateTime("DateTime", productInfo.creationInfo[i].dateTime, creationInfoXML);
        if (!six::Init::isUndefined(productInfo.creationInfo[i].site))
        {
            createString("Site", productInfo.creationInfo[i].site, creationInfoXML);
        }
        mCommon.addParameters("Parameter", getDefaultURI(), productInfo.creationInfo[i].parameter, creationInfoXML);
    }
    mCommon.addParameters("Parameter", getDefaultURI(), productInfo.parameter, productInfoXML);
    return productInfoXML;
}

XMLElem CPHDXMLControl::toXML(const GeoInfo& geoInfo, XMLElem parent)
{
    XMLElem geoInfoXML = newElement("GeoInfo", parent);
    setAttribute(geoInfoXML, "name", geoInfo.getName());

    mCommon.addParameters("Desc", getDefaultURI(), geoInfo.desc, geoInfoXML);
    for(size_t i = 0; i < geoInfo.point.size(); ++i)
    {
        mCommon.createLatLon("Point", geoInfo.point[i], geoInfoXML);
    }
    for(size_t i = 0; i < geoInfo.line.size(); ++i)
    {
        XMLElem lineXML = newElement("Line", geoInfoXML);
        setAttribute(lineXML, "size", six::toString(geoInfo.line[i].numEndpoints));
        for (size_t j = 0; j < geoInfo.line[i].endpoint.size(); ++j)
        {
            XMLElem endptXML = mCommon.createLatLon("Endpoint", static_cast<LatLon>(geoInfo.line[i].endpoint[j]), lineXML);
            setAttribute(endptXML, "index", six::toString(j+1));
        }
    }
    for(size_t i = 0; i < geoInfo.polygon.size(); ++i)
    {
        XMLElem polygonXML = newElement("Polygon", geoInfoXML);
        setAttribute(polygonXML, "size", six::toString(geoInfo.polygon[i].numVertices));
        for (size_t j = 0; j < geoInfo.polygon[i].vertex.size(); ++j)
        {
            XMLElem vertexXML = mCommon.createLatLon("Vertex", static_cast<LatLon>(geoInfo.polygon[i].vertex[j]), polygonXML);
            setAttribute(vertexXML, "index", six::toString(j+1));
        }
    }
    for(size_t i = 0; i < geoInfo.geoInfo.size(); ++i)
    {
        toXML(geoInfo.geoInfo[i], geoInfoXML);
    }
    return geoInfoXML;
}

XMLElem CPHDXMLControl::toXML(const MatchInfo& matchInfo, XMLElem parent)
{
    XMLElem matchInfoXML = newElement("MatchInfo", parent);
    createInt("NumMatchTypes", matchInfo.numMatchTypes, matchInfoXML);
    for (size_t i = 0; i < matchInfo.matchType.size(); ++i)
    {
        XMLElem matchTypeXML = newElement("MatchType", matchInfoXML);
        setAttribute(matchTypeXML, "index", six::toString(matchInfo.matchType[i].index));
        createString("TypeID", matchInfo.matchType[i].typeID, matchTypeXML);
        if (!six::Init::isUndefined(matchInfo.matchType[i].currentIndex))
        {
            createInt("CurrentIndex", matchInfo.matchType[i].currentIndex, matchTypeXML);
        }
        createInt("NumMatchCollections", matchInfo.matchType[i].numMatchCollections, matchTypeXML);
        for(size_t j = 0; j < matchInfo.matchType[i].matchCollection.size(); ++j)
        {
            XMLElem matchCollectionXML = newElement("MatchCollection", matchTypeXML);
            setAttribute(matchCollectionXML, "index", six::toString(matchInfo.matchType[i].matchCollection[j].index));
            createString("CoreName", matchInfo.matchType[i].matchCollection[j].coreName, matchCollectionXML);
            if(!six::Init::isUndefined(matchInfo.matchType[i].matchCollection[j].matchIndex))
            {
                createInt("MatchIndex", matchInfo.matchType[i].matchCollection[j].matchIndex, matchCollectionXML);
            }
            mCommon.addParameters("Parameter", matchInfo.matchType[i].matchCollection[j].parameter, matchCollectionXML);
        }
    }
    return matchInfoXML;
}

/*
 * FROM XML
 */

// TODO: Base
std::auto_ptr<Metadata> CPHDXMLControl::fromXML(const std::string& xmlString)
{
    io::StringStream stringStream;
    stringStream.write(xmlString.c_str(), xmlString.size());
    xml::lite::MinidomParser parser;
    parser.parse(stringStream);
    return fromXML(parser.getDocument());
}

std::auto_ptr<Metadata> CPHDXMLControl::fromXML(const xml::lite::Document* doc)
{
    std::auto_ptr<Metadata> cphd(new Metadata());

    if(!getSchemaPaths().empty()) {
        // Validate schema
        validate(doc, getSchemaPaths(), log());
    }

    XMLElem root = doc->getRootElement();

    XMLElem collectionIDXML   = getFirstAndOnly(root, "CollectionID");
    XMLElem globalXML         = getFirstAndOnly(root, "Global");
    XMLElem sceneCoordsXML    = getFirstAndOnly(root, "SceneCoordinates");
    XMLElem dataXML           = getFirstAndOnly(root, "Data");
    XMLElem channelXML          = getFirstAndOnly(root, "Channel");
    XMLElem pvpXML            = getFirstAndOnly(root, "PVP");
    XMLElem dwellXML          = getFirstAndOnly(root, "Dwell");
    XMLElem refGeoXML         = getFirstAndOnly(root, "ReferenceGeometry");
    XMLElem supportArrayXML   = getOptional(root, "SupportArray");
    XMLElem antennaXML        = getOptional(root, "Antenna");
    XMLElem txRcvXML          = getOptional(root, "TxRcv");
    XMLElem errParamXML       = getOptional(root, "ErrorParameter");
    XMLElem productInfoXML    = getOptional(root, "ProductInfo");
    XMLElem matchInfoXML      = getOptional(root, "MatchInfo");

    std::vector<XMLElem> geoInfoXMLVec;
    root->getElementsByTagName("GeoInfo", geoInfoXMLVec);
    cphd->geoInfo.resize(geoInfoXMLVec.size());

    // Parse XML for each section
    fromXML(collectionIDXML, cphd->collectionID);
    fromXML(globalXML, cphd->global);
    fromXML(sceneCoordsXML, cphd->sceneCoordinates);
    fromXML(dataXML, cphd->data);
    fromXML(channelXML, cphd->channel);
    fromXML(pvpXML, cphd->pvp);
    fromXML(dwellXML, cphd->dwell);
    fromXML(refGeoXML, cphd->referenceGeometry);

    if(supportArrayXML)
    {
        cphd->supportArray.reset(new SupportArray());
        fromXML(supportArrayXML, *(cphd->supportArray));
    }
    if(antennaXML)
    {
        cphd->antenna.reset(new Antenna());
        fromXML(antennaXML, *(cphd->antenna));
    }
    if(txRcvXML)
    {
        cphd->txRcv.reset(new TxRcv());
        fromXML(txRcvXML, *(cphd->txRcv));
    }
    if(errParamXML)
    {
        cphd->errorParameters.reset(new ErrorParameters());
        fromXML(errParamXML, *(cphd->errorParameters));
    }
    if(productInfoXML)
    {
        cphd->productInfo.reset(new ProductInfo());
        fromXML(productInfoXML, *(cphd->productInfo));
    }
    if(matchInfoXML)
    {
        cphd->matchInfo.reset(new MatchInfo());
        fromXML(matchInfoXML, *(cphd->matchInfo));
    }

    for (size_t i = 0; i < geoInfoXMLVec.size(); ++i)
    {
        fromXML(geoInfoXMLVec[i], cphd->geoInfo[i]);
    }

    return cphd;
}

std::auto_ptr<Metadata> CPHDXMLControl::fromXML(const xml::lite::Document* doc, std::vector<std::string>& nodeNames)
{
    std::auto_ptr<Metadata> cphd(new Metadata());
    if(!getSchemaPaths().empty()) {
        // Validate schema
        validate(doc, getSchemaPaths(), log());
    }

    XMLElem root = doc->getRootElement();

    for(size_t i = 0; i < nodeNames.size(); ++i)
    {
        if(nodeNames[i] == "CollectionID") {
            XMLElem collectionIDXML   = getFirstAndOnly(root, "CollectionID");
            fromXML(collectionIDXML, cphd->collectionID);
        }
        else if(nodeNames[i] == "Global") {
            XMLElem globalXML   = getFirstAndOnly(root, "Global");
            fromXML(globalXML, cphd->global);
        }
        else if(nodeNames[i] == "SceneCoordinates") {
            XMLElem sceneCoordsXML   = getFirstAndOnly(root, "SceneCoordinates");
            fromXML(sceneCoordsXML, cphd->sceneCoordinates);
        }
        else if(nodeNames[i] == "Data") {
            XMLElem dataXML   = getFirstAndOnly(root, "Data");
            fromXML(dataXML, cphd->data);
        }
        else if(nodeNames[i] == "Channel") {
            XMLElem channelXML   = getFirstAndOnly(root, "Channel");
            fromXML(channelXML, cphd->channel);
        }
        else if(nodeNames[i] == "PVP") {
            XMLElem pvpXML   = getFirstAndOnly(root, "PVP");
            fromXML(pvpXML, cphd->pvp);
        }
        else if(nodeNames[i] == "Dwell") {
            XMLElem dwellXML   = getFirstAndOnly(root, "Dwell");
            fromXML(dwellXML, cphd->dwell);
        }
        else if(nodeNames[i] == "ReferenceGeometry") {
            XMLElem refGeoXML   = getFirstAndOnly(root, "ReferenceGeometry");
            fromXML(refGeoXML, cphd->referenceGeometry);
        }
        else if(nodeNames[i] == "SupportArray") {
            XMLElem supportArrayXML   = getFirstAndOnly(root, "SupportArray");
            cphd->supportArray.reset(new SupportArray());
            fromXML(supportArrayXML, *(cphd->supportArray));
        }
        else if(nodeNames[i] == "Antenna") {
            XMLElem antennaXML   = getFirstAndOnly(root, "Antenna");
            cphd->antenna.reset(new Antenna());
            fromXML(antennaXML, *(cphd->antenna));
        }
        else if(nodeNames[i] == "TxRcv") {
            XMLElem txRcvXML   = getFirstAndOnly(root, "TxRcv");
            cphd->txRcv.reset(new TxRcv());
            fromXML(txRcvXML, *(cphd->txRcv));
        }
        else if(nodeNames[i] == "ErrorParameters") {
            XMLElem errParamXML   = getFirstAndOnly(root, "ErrorParameters");
            cphd->errorParameters.reset(new ErrorParameters());
            fromXML(errParamXML, *(cphd->errorParameters));
        }
        else if(nodeNames[i] == "ProductInfo") {
            XMLElem productInfoXML   = getFirstAndOnly(root, "ProductInfo");
            cphd->productInfo.reset(new ProductInfo());
            fromXML(productInfoXML, *(cphd->productInfo));
        }
        else if(nodeNames[i] == "GeoInfo") {
            std::vector<XMLElem> geoInfoXMLVec;
            root->getElementsByTagName("GeoInfo", geoInfoXMLVec);
            cphd->geoInfo.resize(geoInfoXMLVec.size());
            for(size_t j = 0; j < geoInfoXMLVec.size(); ++j)
            {
                fromXML(geoInfoXMLVec[j], cphd->geoInfo[j]);
            }
        }
        else if(nodeNames[i] == "MatchInfo") {
            XMLElem matchInfoXML   = getFirstAndOnly(root, "MatchInfo");
            cphd->matchInfo.reset(new MatchInfo());
            fromXML(matchInfoXML, *(cphd->matchInfo));
        }
        else {
            throw except::Exception(Ctxt(
                    "Invalid node name provided"));
        }
    }
    return cphd;
}

void CPHDXMLControl::fromXML(const XMLElem collectionIDXML, CollectionID& collectionID)
{

    parseString(getFirstAndOnly(collectionIDXML, "CollectorName"),
                collectionID.collectorName);

    XMLElem element = getOptional(collectionIDXML, "IlluminatorName");
    if (element)
        parseString(element, collectionID.illuminatorName);

    element = getOptional(collectionIDXML, "CoreName");
    if (element)
        parseString(element, collectionID.coreName);

    element = getOptional(collectionIDXML, "CollectType");
    if (element)
        collectionID.collectType
                = six::toType<six::CollectType>(element->getCharacterData());

    XMLElem radarModeXML = getFirstAndOnly(collectionIDXML, "RadarMode");

    collectionID.radarMode
            = six::toType<six::RadarModeType>(getFirstAndOnly(radarModeXML,
                                              "ModeType")->getCharacterData());

    element = getOptional(radarModeXML, "ModeID");
    if (element)
        parseString(element, collectionID.radarModeID);

    std::string classification;
    parseString(getFirstAndOnly(collectionIDXML, "Classification"),
                classification);
    collectionID.setClassificationLevel(classification);

    element = getFirstAndOnly(collectionIDXML, "ReleaseInfo");
    parseString(element, collectionID.releaseInfo);

    // Optional
    std::vector<std::string> countryCodes;
    element = getOptional(collectionIDXML, "CountryCode");
    if (element)
    {
        std::string countryCodeStr;
        parseString(element, countryCodeStr);
        collectionID.countryCodes = str::split(countryCodeStr, ",");
        for (size_t ii = 0; ii < collectionID.countryCodes.size(); ++ii)
        {
            str::trim(collectionID.countryCodes[ii]);
        }
    }

    //optional
    mCommon.parseParameters(collectionIDXML, "Parameter", collectionID.parameters);
}

void CPHDXMLControl::fromXML(const XMLElem globalXML, Global& global)
{
    global.domainType = DomainType(
            getFirstAndOnly(globalXML, "DomainType")->getCharacterData());
    global.sgn = PhaseSGN(
            getFirstAndOnly(globalXML, "SGN")->getCharacterData());

    // Timeline
    const XMLElem timelineXML = getFirstAndOnly(globalXML, "Timeline");
    parseDateTime(
            getFirstAndOnly(timelineXML, "CollectionStart"),
            global.timeline.collectionStart);

    // Optional
    const XMLElem rcvCollectionXML = getOptional(timelineXML,
                                                 "RcvCollectionStart");
    if (rcvCollectionXML)
    {
        parseDateTime(rcvCollectionXML,
                      global.timeline.rcvCollectionStart);
    }

    parseDouble(
            getFirstAndOnly(timelineXML, "TxTime1"), global.timeline.txTime1);
    parseDouble(
            getFirstAndOnly(timelineXML, "TxTime2"), global.timeline.txTime2);

    // FxBand
    const XMLElem fxBandXML = getFirstAndOnly(globalXML, "FxBand");
    parseDouble(getFirstAndOnly(fxBandXML, "FxMin"), global.fxBand.fxMin);
    parseDouble(getFirstAndOnly(fxBandXML, "FxMax"), global.fxBand.fxMax);

    // TOASwath
    const XMLElem toaSwathXML = getFirstAndOnly(globalXML, "TOASwath");
    parseDouble(getFirstAndOnly(toaSwathXML, "TOAMin"), global.toaSwath.toaMin);
    parseDouble(getFirstAndOnly(toaSwathXML, "TOAMax"), global.toaSwath.toaMax);

    // TropoParameters
    const XMLElem tropoXML = getOptional(globalXML, "TropoParameters");
    if (tropoXML)
    {
        // Optional
        global.tropoParameters.reset(new TropoParameters());
        parseDouble(getFirstAndOnly(tropoXML, "N0"), global.tropoParameters->n0);
        global.tropoParameters->refHeight =
                getFirstAndOnly(tropoXML, "RefHeight")->getCharacterData();
    }

    // IonoParameters
    const XMLElem ionoXML = getOptional(globalXML, "IonoParameters");
    if (tropoXML)
    {
        // Optional
        global.ionoParameters.reset(new IonoParameters());
        parseDouble(getFirstAndOnly(ionoXML, "TECV"), global.ionoParameters->tecv);
        parseDouble(getFirstAndOnly(ionoXML, "F2Height"), global.ionoParameters->f2Height);
    }
}

void CPHDXMLControl::fromXML(const XMLElem sceneCoordsXML,
                             SceneCoordinates& scene)
{
    scene.earthModel = EarthModelType(
            getFirstAndOnly(sceneCoordsXML, "EarthModel")->getCharacterData());

    // IARP
    const XMLElem iarpXML = getFirstAndOnly(sceneCoordsXML, "IARP");
    mCommon.parseVector3D(getFirstAndOnly(iarpXML, "ECF"), scene.iarp.ecf);
    mCommon.parseLatLonAlt(getFirstAndOnly(iarpXML, "LLH"), scene.iarp.llh);

    // ReferenceSurface
    const XMLElem surfaceXML = getFirstAndOnly(sceneCoordsXML, "ReferenceSurface");
    const XMLElem planarXML = getOptional(surfaceXML, "Planar");
    const XMLElem haeXML = getOptional(surfaceXML, "HAE");
    if (planarXML && !haeXML)
    {
        // Choice type
        scene.referenceSurface.planar.reset(new Planar());
        mCommon.parseVector3D(getFirstAndOnly(planarXML, "uIAX"),
                              scene.referenceSurface.planar->uIax);
        mCommon.parseVector3D(getFirstAndOnly(planarXML, "uIAY"),
                              scene.referenceSurface.planar->uIay);
    }
    else if (haeXML && !planarXML)
    {
        // Choice type
        scene.referenceSurface.hae.reset(new HAE());
        mCommon.parseLatLon(getFirstAndOnly(haeXML, "uIAXLL"),
                            scene.referenceSurface.hae->uIax);
        mCommon.parseLatLon(getFirstAndOnly(haeXML, "uIAYLL"),
                            scene.referenceSurface.hae->uIay);
    }
    else
    {
        throw except::Exception(Ctxt(
                "ReferenceSurface must exactly one of Planar or HAE element"));
    }

    // ImageArea
    const XMLElem imageAreaXML = getFirstAndOnly(sceneCoordsXML, "ImageArea");
    parseAreaType(imageAreaXML, scene.imageArea);

    // ImageAreaCorners
    const XMLElem cornersXML = getFirstAndOnly(sceneCoordsXML,
                                               "ImageAreaCornerPoints");
    mCommon.parseFootprint(cornersXML, "IACP", scene.imageAreaCorners);

    // Extended Area
    const XMLElem extendedAreaXML = getOptional(sceneCoordsXML, "ExtendedArea");
    if (extendedAreaXML)
    {
        scene.extendedArea.reset(new AreaType());
        parseAreaType(extendedAreaXML, *scene.extendedArea);
    }

    // Image Grid
    const XMLElem gridXML = getOptional(sceneCoordsXML, "ImageGrid");
    if (gridXML)
    {
        // Optional
        scene.imageGrid.reset(new ImageGrid());
        const XMLElem identifierXML = getOptional(gridXML, "Identifier");
        if (identifierXML)
        {
            parseString(identifierXML, scene.imageGrid->identifier);
        }
        parseLineSample(getFirstAndOnly(gridXML, "IARPLocation"),
                        scene.imageGrid->iarpLocation);
        parseIAExtent(getFirstAndOnly(gridXML, "IAXExtent"),
                      scene.imageGrid->xExtent);
        parseIAExtent(getFirstAndOnly(gridXML, "IAYExtent"),
                      scene.imageGrid->yExtent);

        // Segment List
        const XMLElem segListXML = getOptional(gridXML, "SegmentList");
        if (segListXML)
        {
            // Optional
            size_t numSegments;
            parseUInt(getFirstAndOnly(segListXML, "NumSegments"), numSegments);
            scene.imageGrid->segments.resize(numSegments);

            std::vector<XMLElem> segmentsXML;
            segListXML->getElementsByTagName("Segment", segmentsXML);

            for (size_t ii = 0; ii < segmentsXML.size(); ++ii)
            {
                const XMLElem segmentXML = segmentsXML[ii];
                parseString(getFirstAndOnly(segmentXML, "Identifier"),
                         scene.imageGrid->segments[ii].identifier);
                parseInt(getFirstAndOnly(segmentXML, "StartLine"),
                         scene.imageGrid->segments[ii].startLine);
                parseInt(getFirstAndOnly(segmentXML, "StartSample"),
                         scene.imageGrid->segments[ii].startSample);
                parseInt(getFirstAndOnly(segmentXML, "EndLine"),
                         scene.imageGrid->segments[ii].endLine);
                parseInt(getFirstAndOnly(segmentXML, "EndSample"),
                         scene.imageGrid->segments[ii].endSample);

                const XMLElem polygonXML = getOptional(segmentXML,
                                                       "SegmentPolygon");
                if (polygonXML)
                {
                    // Optional
                    sscanf(polygonXML->attribute("size").c_str(), "%zu", &scene.imageGrid->segments[ii].size);
                    std::vector<XMLElem> polyVertices;
                    polygonXML->getElementsByTagName("SV", polyVertices);
                    if (polyVertices.size() < 3)
                    {
                        throw except::Exception(Ctxt(
                                "Polygon must have at least 3 vertices"));
                    }
                    std::vector<LineSample>& vertices =
                            scene.imageGrid->segments[ii].polygon;
                    vertices.resize(polyVertices.size());
                    for (size_t jj = 0; jj < polyVertices.size(); ++jj)
                    {
                        size_t tempIdx;
                        sscanf(polyVertices[jj]->attribute("index").c_str(), "%zu", &tempIdx);
                        vertices[jj].setIndex(tempIdx);
                        parseLineSample(polyVertices[jj], vertices[jj]);
                    }
                }
            }
        }
    }
}

void CPHDXMLControl::fromXML(const XMLElem dataXML, Data& data)
{
    const XMLElem signalXML = getFirstAndOnly(dataXML, "SignalArrayFormat");
    data.signalArrayFormat = SignalArrayFormat(signalXML->getCharacterData());

    size_t numBytesPVP_temp;
    XMLElem numBytesPVPXML = getFirstAndOnly(dataXML, "NumBytesPVP");
    parseUInt(numBytesPVPXML, numBytesPVP_temp);
    if(numBytesPVP_temp % 8 != 0)
        {
            throw except::Exception(Ctxt(
                    "Number of bytes must be multiple of 8"));
        }
    data.numBytesPVP = numBytesPVP_temp;
    // Channels
    std::vector<XMLElem> channelsXML;
    dataXML->getElementsByTagName("Channel", channelsXML);
    data.channels.resize(channelsXML.size());
    for (size_t ii = 0; ii < channelsXML.size(); ++ii)
    {
        parseString(getFirstAndOnly(channelsXML[ii], "Identifier"),
                    data.channels[ii].identifier);
        parseUInt(getFirstAndOnly(channelsXML[ii], "NumVectors"),
                  data.channels[ii].numVectors);
        parseUInt(getFirstAndOnly(channelsXML[ii], "NumSamples"),
                  data.channels[ii].numSamples);
        parseUInt(getFirstAndOnly(channelsXML[ii], "SignalArrayByteOffset"),
                  data.channels[ii].signalArrayByteOffset);
        parseUInt(getFirstAndOnly(channelsXML[ii], "PVPArrayByteOffset"),
                  data.channels[ii].pvpArrayByteOffset);
        const XMLElem compressionSizeXML = getOptional(channelsXML[ii],
                                                       "CompressedSignalSize");
        if (compressionSizeXML)
        {
            // Optional
            parseUInt(compressionSizeXML,
                      data.channels[ii].compressedSignalSize);
        }
    }

    parseString(getFirstAndOnly(dataXML, "SignalCompressionID"),
                data.signalCompressionID);

    // Support Arrays
    std::vector<XMLElem> supportsXML;
    dataXML->getElementsByTagName("SupportArray", supportsXML);
    data.supportArrays.resize(supportsXML.size());
    for (size_t ii = 0; ii < supportsXML.size(); ++ii)
    {
        parseString(getFirstAndOnly(supportsXML[ii], "Identifier"),
                                    data.supportArrays[ii].identifier);
        parseUInt(getFirstAndOnly(supportsXML[ii], "NumRows"),
                                  data.supportArrays[ii].numRows);
        parseUInt(getFirstAndOnly(supportsXML[ii], "NumCols"),
                                  data.supportArrays[ii].numCols);
        parseUInt(getFirstAndOnly(supportsXML[ii], "BytesPerElement"),
                                  data.supportArrays[ii].bytesPerElement);
        parseUInt(getFirstAndOnly(supportsXML[ii], "ArrayByteOffset"),
                                  data.supportArrays[ii].arrayByteOffset);
    }
}

void CPHDXMLControl::fromXML(const XMLElem channelXML, Channel& channel)
{
    parseString(getFirstAndOnly(channelXML, "RefChId"), channel.refChId);
    parseBooleanType(getFirstAndOnly(channelXML, "FXFixedCPHD"),
                     channel.fxFixedCphd);
    parseBooleanType(getFirstAndOnly(channelXML, "TOAFixedCPHD"),
                     channel.toaFixedCphd);
    parseBooleanType(getFirstAndOnly(channelXML, "SRPFixedCPHD"),
                     channel.srpFixedCphd);

    std::vector<XMLElem> parametersXML;
    channelXML->getElementsByTagName("Parameters", parametersXML);
    channel.parameters.resize(parametersXML.size());
    for (size_t ii = 0; ii < parametersXML.size(); ++ii)
    {
        parseChannelParameters(parametersXML[ii], channel.parameters[ii]);
    }

    XMLElem addedParametersXML = getOptional(channelXML, "AddedParameters");
    if(addedParametersXML)
    {
        mCommon.parseParameters(addedParametersXML, "Parameter", channel.addedParameters);
    }
}

void CPHDXMLControl::fromXML(const XMLElem pvpXML, Pvp& pvp)
{
    std::vector<XMLElem> pvpTypeXML;
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "TxTime"), pvp.txTime);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "TxPos"), pvp.txPos);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "TxVel"), pvp.txVel);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "RcvTime"), pvp.rcvTime);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "RcvPos"), pvp.rcvPos);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "RcvVel"), pvp.rcvVel);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "SRPPos"), pvp.srpPos);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "aFDOP"), pvp.aFDOP);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "aFRR1"), pvp.aFRR1);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "aFRR2"), pvp.aFRR2);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "FX1"), pvp.fx1);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "FX2"), pvp.fx2);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "TOA1"), pvp.toa1);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "TOA2"), pvp.toa2);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "TDTropoSRP"), pvp.tdTropoSRP);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "SC0"), pvp.sc0);
    parsePVPType(pvp, getFirstAndOnly(pvpXML, "SCSS"), pvp.scss);
    const XMLElem AmpSFXML = getOptional(pvpXML, "AmpSF");
    if (AmpSFXML)
    {
        pvp.ampSF.reset(new PVPType());
        parsePVPType(pvp, AmpSFXML, *(pvp.ampSF));
    }

    const XMLElem FXN1XML = getOptional(pvpXML, "FXN1");
    if (FXN1XML)
    {
        pvp.fxN1.reset(new PVPType());
        parsePVPType(pvp, FXN1XML, *(pvp.fxN1));
    }

    const XMLElem FXN2XML = getOptional(pvpXML, "FXN2");
    if (FXN2XML)
    {
        pvp.fxN2.reset(new PVPType());
        parsePVPType(pvp, FXN2XML, *(pvp.fxN2));
    }

    const XMLElem TOAE1XML = getOptional(pvpXML, "TOAE1");
    if (TOAE1XML)
    {
        pvp.toaE1.reset(new PVPType());
        parsePVPType(pvp, TOAE1XML, *(pvp.toaE1));
    }

    const XMLElem TOAE2XML = getOptional(pvpXML, "TOAE2");
    if (TOAE2XML)
    {
        pvp.toaE2.reset(new PVPType());
        parsePVPType(pvp, TOAE2XML, *(pvp.toaE2));
    }

    const XMLElem TDIonoSRPXML = getOptional(pvpXML, "TDIonoSRP");
    if (TDIonoSRPXML)
    {
        pvp.tdIonoSRP.reset(new PVPType());
        parsePVPType(pvp, TDIonoSRPXML, *(pvp.tdIonoSRP));
    }

    const XMLElem SIGNALXML = getOptional(pvpXML, "SIGNAL");
    if (SIGNALXML)
    {
        pvp.signal.reset(new PVPType());
        parsePVPType(pvp, SIGNALXML, *(pvp.signal));
    }

    std::vector<XMLElem> addedParamsXML;
    pvpXML->getElementsByTagName("AddedPVP", addedParamsXML);
    if(addedParamsXML.empty())
    {
        return;
    }
    pvp.setNumAddedParameters(addedParamsXML.size());
    for (size_t ii = 0; ii < addedParamsXML.size(); ++ii)
    {
        parsePVPType(pvp, addedParamsXML[ii], ii);
    }

}

void CPHDXMLControl::fromXML(const XMLElem DwellXML,
                             Dwell& dwell)
{
    // CODTime
    parseUInt(getFirstAndOnly(DwellXML, "NumCODTimes"), dwell.numCODTimes);
    dwell.cod.resize(dwell.numCODTimes);

    std::vector<XMLElem> CODXML_vec;
    DwellXML->getElementsByTagName("CODTime", CODXML_vec);
    dwell.cod.resize(dwell.numCODTimes);
    for(size_t ii = 0; ii < CODXML_vec.size(); ++ii) 
    {
        parseString(getFirstAndOnly(CODXML_vec[ii], "Identifier"), dwell.cod[ii].identifier);
        mCommon.parsePoly2D(getFirstAndOnly(CODXML_vec[ii], "CODTimePoly"), dwell.cod[ii].codTimePoly);
    }

    // DwellTime
    parseUInt(getFirstAndOnly(DwellXML, "NumDwellTimes"), dwell.numDwellTimes);
    dwell.dtime.resize(dwell.numDwellTimes);

    std::vector<XMLElem> dtimeXML_vec;
    DwellXML->getElementsByTagName("DwellTime", dtimeXML_vec);
    dwell.dtime.resize(dwell.numDwellTimes);
    for(size_t ii = 0; ii < dtimeXML_vec.size(); ++ii) 
    {
        parseString(getFirstAndOnly(dtimeXML_vec[ii], "Identifier"), dwell.dtime[ii].identifier);
        mCommon.parsePoly2D(getFirstAndOnly(dtimeXML_vec[ii], "DwellTimePoly"), dwell.dtime[ii].dwellTimePoly); 
    }
}

void CPHDXMLControl::fromXML(const XMLElem refGeoXML, ReferenceGeometry& refGeo)
{
    XMLElem srpXML = getFirstAndOnly(refGeoXML, "SRP");
    mCommon.parseVector3D(getFirstAndOnly(srpXML, "ECF"), refGeo.srp.ecf);
    mCommon.parseVector3D(getFirstAndOnly(srpXML, "IAC"), refGeo.srp.iac);

    parseDouble(getFirstAndOnly(refGeoXML, "ReferenceTime"), refGeo.referenceTime);
    parseDouble(getFirstAndOnly(refGeoXML, "SRPCODTime"), refGeo.srpCODTime);
    parseDouble(getFirstAndOnly(refGeoXML, "SRPDwellTime"), refGeo.srpDwellTime);

    const XMLElem monoXML = getOptional(refGeoXML, "Monostatic");
    const XMLElem biXML = getOptional(refGeoXML, "Bistatic");

    if (monoXML && !biXML)
    {
        refGeo.monostatic.reset(new Monostatic());
        parseCommon(monoXML, (ImagingType*)refGeo.monostatic.get());
        parseDouble(getFirstAndOnly(monoXML, "SlantRange"), refGeo.monostatic->slantRange);
        parseDouble(getFirstAndOnly(monoXML, "GroundRange"), refGeo.monostatic->groundRange);
        parseDouble(getFirstAndOnly(monoXML, "DopplerConeAngle"), refGeo.monostatic->dopplerConeAngle);
        parseDouble(getFirstAndOnly(monoXML, "IncidenceAngle"), refGeo.monostatic->incidenceAngle);
        mCommon.parseVector3D(getFirstAndOnly(monoXML, "ARPPos"), refGeo.monostatic->arpPos);
        mCommon.parseVector3D(getFirstAndOnly(monoXML, "ARPVel"), refGeo.monostatic->arpVel);
        std::string side = "";
        parseString(getFirstAndOnly(monoXML, "SideOfTrack"), side);
        refGeo.monostatic->sideOfTrack = (side == "L" ? six::SideOfTrackType("LEFT") : six::SideOfTrackType("RIGHT"));

    }
    else if (!monoXML && biXML)
    {
        refGeo.bistatic.reset(new Bistatic());
        parseCommon(biXML, (ImagingType*)refGeo.bistatic.get());
        parseDouble(getFirstAndOnly(biXML, "AzimuthAngleRate"), refGeo.bistatic->azimuthAngleRate);
        parseDouble(getFirstAndOnly(biXML, "BistaticAngle"), refGeo.bistatic->bistaticAngle);
        parseDouble(getFirstAndOnly(biXML, "BistaticAngleRate"), refGeo.bistatic->bistaticAngleRate);

        parsePlatformParams(getFirstAndOnly(biXML, "TxPlatform"), refGeo.bistatic->txPlatform);
        parsePlatformParams(getFirstAndOnly(biXML, "RcvPlatform"), refGeo.bistatic->rcvPlatform);
    }
    else
    {
        throw except::Exception(Ctxt(
                "ReferenceGeometry must be exactly one of Monostatic or Bistatic element"));
    }

}

void CPHDXMLControl::fromXML(const XMLElem supportArrayXML, SupportArray& supportArray)
{
    std::vector<XMLElem> iazArrayXMLVec;
    supportArrayXML->getElementsByTagName("IAZArray", iazArrayXMLVec);
    supportArray.iazArray.resize(iazArrayXMLVec.size());
    for (size_t i = 0; i < iazArrayXMLVec.size(); ++i)
    {
        parseSupportArrayParameter(iazArrayXMLVec[i], supportArray.iazArray[i], false);
    }

    std::vector<XMLElem> antGainPhaseXMLVec;
    supportArrayXML->getElementsByTagName("AntGainPhase", antGainPhaseXMLVec);
    supportArray.antGainPhase.resize(antGainPhaseXMLVec.size());
    for (size_t i = 0; i < antGainPhaseXMLVec.size(); ++i)
    {
        parseSupportArrayParameter(antGainPhaseXMLVec[i], supportArray.antGainPhase[i], false);
    }

    std::vector<XMLElem> addedSupportArrayXMLVec;
    supportArrayXML->getElementsByTagName("AddedSupportArray", addedSupportArrayXMLVec);
    supportArray.addedSupportArray.resize(addedSupportArrayXMLVec.size());
    for (size_t i = 0; i < addedSupportArrayXMLVec.size(); ++i)
    {
        parseSupportArrayParameter(addedSupportArrayXMLVec[i], supportArray.addedSupportArray[i], true);
        parseString(getFirstAndOnly(addedSupportArrayXMLVec[i], "Identifier"), supportArray.addedSupportArray[i].identifier);
        parseString(getFirstAndOnly(addedSupportArrayXMLVec[i], "XUnits"), supportArray.addedSupportArray[i].xUnits);
        parseString(getFirstAndOnly(addedSupportArrayXMLVec[i], "YUnits"), supportArray.addedSupportArray[i].yUnits);
        parseString(getFirstAndOnly(addedSupportArrayXMLVec[i], "ZUnits"), supportArray.addedSupportArray[i].zUnits);
        mCommon.parseParameters(addedSupportArrayXMLVec[i], "Parameter", supportArray.addedSupportArray[i].parameter);
    }
}


void CPHDXMLControl::fromXML(const XMLElem antennaXML, Antenna& antenna)
{
    parseUInt(getFirstAndOnly(antennaXML, "NumACFs"), antenna.numACFs);
    parseUInt(getFirstAndOnly(antennaXML, "NumAPCs"), antenna.numAPCs);
    parseUInt(getFirstAndOnly(antennaXML, "NumAntPats"), antenna.numAntPats);
    //! Parse AntCoordFrame
    std::vector<XMLElem> antCoordFrameXMLVec;
    antennaXML->getElementsByTagName("AntCoordFrame", antCoordFrameXMLVec);
    antenna.antCoordFrame.resize(antCoordFrameXMLVec.size());
    for( size_t i = 0; i < antCoordFrameXMLVec.size(); ++i)
    {
        parseString(getFirstAndOnly(antCoordFrameXMLVec[i], "Identifier"), antenna.antCoordFrame[i].identifier);
        mCommon.parsePolyXYZ(getFirstAndOnly(antCoordFrameXMLVec[i], "XAxisPoly"), antenna.antCoordFrame[i].xAxisPoly);
        mCommon.parsePolyXYZ(getFirstAndOnly(antCoordFrameXMLVec[i], "YAxisPoly"), antenna.antCoordFrame[i].yAxisPoly);
    }

    //! Parse AntPhaseCenter
    std::vector<XMLElem> antPhaseCenterXMLVec;
    antennaXML->getElementsByTagName("AntPhaseCenter", antPhaseCenterXMLVec);
    antenna.antPhaseCenter.resize(antPhaseCenterXMLVec.size());
    for( size_t i = 0; i < antPhaseCenterXMLVec.size(); ++i)
    {
        parseString(getFirstAndOnly(antPhaseCenterXMLVec[i], "Identifier"), antenna.antPhaseCenter[i].identifier);
        parseString(getFirstAndOnly(antPhaseCenterXMLVec[i], "ACFId"), antenna.antPhaseCenter[i].acfId);
        mCommon.parseVector3D(getFirstAndOnly(antPhaseCenterXMLVec[i], "APCXYZ"), antenna.antPhaseCenter[i].apcXYZ);
    }

    std::vector<XMLElem> antPatternXMLVec;
    antennaXML->getElementsByTagName("AntPattern", antPatternXMLVec);
    antenna.antPattern.resize(antPatternXMLVec.size());
    for( size_t i = 0; i < antPatternXMLVec.size(); ++i)
    {
        parseString(getFirstAndOnly(antPatternXMLVec[i], "Identifier"), antenna.antPattern[i].identifier);
        parseDouble(getFirstAndOnly(antPatternXMLVec[i], "FreqZero"), antenna.antPattern[i].freqZero);
        XMLElem gainZeroXML = getOptional(antPatternXMLVec[i], "GainZero");
        if(gainZeroXML)
        {
            parseDouble(gainZeroXML, antenna.antPattern[i].gainZero);
        }
        XMLElem ebFreqShiftXML = getOptional(antPatternXMLVec[i], "EBFreqShift");
        if(ebFreqShiftXML)
        {
            parseBooleanType(ebFreqShiftXML, antenna.antPattern[i].ebFreqShift);
        }
        XMLElem mlFreqDilationXML = getOptional(antPatternXMLVec[i], "MLFreqDilation");
        if(mlFreqDilationXML)
        {
            parseBooleanType(mlFreqDilationXML, antenna.antPattern[i].mlFreqDilation);
        }
        XMLElem gainBSPoly = getOptional(antPatternXMLVec[i], "GainBSPoly");
        if(gainBSPoly)
        {
            mCommon.parsePoly1D(gainBSPoly, antenna.antPattern[i].gainBSPoly);
        }

        // Parse EB
        XMLElem ebXML = getFirstAndOnly(antPatternXMLVec[i], "EB");
        mCommon.parsePoly1D(getFirstAndOnly(ebXML, "DCXPoly"), antenna.antPattern[i].eb.dcXPoly);
        mCommon.parsePoly1D(getFirstAndOnly(ebXML, "DCYPoly"), antenna.antPattern[i].eb.dcYPoly);

        // Parse Array
        XMLElem arrayXML = getFirstAndOnly(antPatternXMLVec[i], "Array");
        mCommon.parsePoly2D(getFirstAndOnly(arrayXML, "GainPoly"), antenna.antPattern[i].array.gainPoly);
        mCommon.parsePoly2D(getFirstAndOnly(arrayXML, "PhasePoly"), antenna.antPattern[i].array.phasePoly);

        // Parse Element
        XMLElem elementXML = getFirstAndOnly(antPatternXMLVec[i], "Element");
        mCommon.parsePoly2D(getFirstAndOnly(elementXML, "GainPoly"), antenna.antPattern[i].element.gainPoly);
        mCommon.parsePoly2D(getFirstAndOnly(elementXML, "PhasePoly"), antenna.antPattern[i].element.phasePoly);

        // Parse GainPhaseArray
        std::vector<XMLElem> gainPhaseArrayXMLVec;
        antPatternXMLVec[i]->getElementsByTagName("GainPhaseArray", gainPhaseArrayXMLVec);
        antenna.antPattern[i].gainPhaseArray.resize(gainPhaseArrayXMLVec.size());
        for (size_t j = 0; j < gainPhaseArrayXMLVec.size(); ++j)
        {
            parseDouble(getFirstAndOnly(gainPhaseArrayXMLVec[j], "Freq"),  antenna.antPattern[i].gainPhaseArray[j].freq);
            parseString(getFirstAndOnly(gainPhaseArrayXMLVec[j], "ArrayId"), antenna.antPattern[i].gainPhaseArray[j].arrayId);
            XMLElem elementIdXML = getOptional(gainPhaseArrayXMLVec[j], "ElementId");
            if(elementIdXML)
            {
                parseString(elementIdXML, antenna.antPattern[i].gainPhaseArray[j].elementId);
            }

        }
    }
}

void CPHDXMLControl::fromXML(const XMLElem txRcvXML, TxRcv& txRcv)
{
    parseUInt(getFirstAndOnly(txRcvXML, "NumTxWFs"), txRcv.numTxWFs);
    parseUInt(getFirstAndOnly(txRcvXML, "NumRcvs"), txRcv.numRcvs);
    std::vector<XMLElem> txWFXMLVec;
    txRcvXML->getElementsByTagName("TxWFParameters", txWFXMLVec);
    txRcv.txWFParameters.resize(txWFXMLVec.size());
    for(size_t i = 0; i < txWFXMLVec.size(); ++i)
    {
        parseTxRcvParameter(txWFXMLVec[i], txRcv.txWFParameters[i]);
        parseDouble(getFirstAndOnly(txWFXMLVec[i], "PulseLength"), txRcv.txWFParameters[i].pulseLength);
        parseDouble(getFirstAndOnly(txWFXMLVec[i], "RFBandwidth"), txRcv.txWFParameters[i].rfBandwidth);
        XMLElem powerXML = getOptional(txWFXMLVec[i], "Power");
        if(powerXML)
        {
            parseDouble(powerXML, txRcv.txWFParameters[i].power);
        }
    }

    std::vector<XMLElem> rcvXMLVec;
    txRcvXML->getElementsByTagName("RcvParameters", rcvXMLVec);
    txRcv.rcvParameters.resize(rcvXMLVec.size());
    for(size_t i = 0; i < rcvXMLVec.size(); ++i)
    {
        parseTxRcvParameter(rcvXMLVec[i], txRcv.rcvParameters[i]);
        parseDouble(getFirstAndOnly(rcvXMLVec[i], "WindowLength"), txRcv.rcvParameters[i].windowLength);
        parseDouble(getFirstAndOnly(rcvXMLVec[i], "SampleRate"), txRcv.rcvParameters[i].sampleRate);
        parseDouble(getFirstAndOnly(rcvXMLVec[i], "IFFilterBW"), txRcv.rcvParameters[i].ifFilterBW);
        XMLElem pathGainXML = getOptional(rcvXMLVec[i], "PathGain");
        if(pathGainXML)
        {
            parseDouble(pathGainXML, txRcv.rcvParameters[i].pathGain);
        }
    }

}

void CPHDXMLControl::fromXML(const XMLElem errParamXML, ErrorParameters& errParam)
{
    XMLElem monostaticXML = getOptional(errParamXML, "Monostatic");
    XMLElem bistaticXML = getOptional(errParamXML, "Bistatic");

    if(monostaticXML && !bistaticXML)
    {
        errParam.monostatic.reset(new ErrorParameters::Monostatic());
        parsePosVelErr(getFirstAndOnly(monostaticXML, "PosVelErr"), errParam.monostatic->posVelErr);

        XMLElem radarSensorXML = getFirstAndOnly(monostaticXML, "RadarSensor");
        parseDouble(getFirstAndOnly(radarSensorXML, "RangeBias"), errParam.monostatic->radarSensor.rangeBias);

        XMLElem clockFreqSFXML = getOptional(radarSensorXML, "ClockFreqSF");
        if(clockFreqSFXML)
        {
            parseDouble(clockFreqSFXML, errParam.monostatic->radarSensor.clockFreqSF);
        }

        XMLElem collectionStartTimeXML = getOptional(radarSensorXML, "CollectionStartTime");
        if(collectionStartTimeXML)
        {
            parseDouble(collectionStartTimeXML, errParam.monostatic->radarSensor.collectionStartTime);
        }

        XMLElem rangeBiasDecorrXML = getOptional(radarSensorXML, "RangeBiasDecorr");
        if(rangeBiasDecorrXML)
        {
            errParam.monostatic->radarSensor.rangeBiasDecorr.reset(new Decorr());
            parseDecorr(rangeBiasDecorrXML, *(errParam.monostatic->radarSensor.rangeBiasDecorr));
        }

        XMLElem tropoErrorXML = getFirstAndOnly(monostaticXML, "TropoError");
        if(tropoErrorXML)
        {
            errParam.monostatic->tropoError.reset(new ErrorParameters::Monostatic::TropoError());
            XMLElem verticalXML = getOptional(tropoErrorXML, "TropoRangeVertical");
            if(verticalXML)
            {
                parseDouble(verticalXML, errParam.monostatic->tropoError->tropoRangeVertical);
            }
            XMLElem slantXML = getOptional(tropoErrorXML, "TropoRangeSlant");
            if(slantXML)
            {
                parseDouble(slantXML, errParam.monostatic->tropoError->tropoRangeSlant);
            }
            XMLElem decorrXML = getOptional(tropoErrorXML, "TropoRangeDecorr");
            if(decorrXML)
            {
                errParam.monostatic->tropoError->tropoRangeDecorr.reset(new Decorr());
                parseDecorr(decorrXML, *(errParam.monostatic->tropoError->tropoRangeDecorr));
            }
        }

        XMLElem ionoErrorXML = getFirstAndOnly(monostaticXML, "IonoError");
        if(ionoErrorXML)
        {
            errParam.monostatic->ionoError.reset(new ErrorParameters::Monostatic::IonoError());
            parseDouble(getFirstAndOnly(ionoErrorXML, "IonoRangeVertical"), errParam.monostatic->ionoError->ionoRangeVertical);

            XMLElem rateVerticalXML = getOptional(ionoErrorXML, "IonoRangeRateVertical");
            if(rateVerticalXML)
            {
                parseDouble(rateVerticalXML, errParam.monostatic->ionoError->ionoRangeRateVertical);
            }
            XMLElem rgrgRateCCXML = getOptional(ionoErrorXML, "IonoRgRgRateCC");
            if(rgrgRateCCXML)
            {
                parseDouble(rgrgRateCCXML, errParam.monostatic->ionoError->ionoRgRgRateCC);
            }
            XMLElem decorrXML = getOptional(ionoErrorXML, "IonoRangeVertDecorr");
            if(decorrXML)
            {
                errParam.monostatic->ionoError->ionoRangeVertDecorr.reset(new Decorr());
                parseDecorr(decorrXML, *(errParam.monostatic->ionoError->ionoRangeVertDecorr));
            }
        }
        mCommon.parseParameters(monostaticXML, "Parameter", errParam.monostatic->parameter);
    }
    else if(!monostaticXML && bistaticXML)
    {
        errParam.bistatic.reset(new ErrorParameters::Bistatic());
        parsePlatform(getFirstAndOnly(bistaticXML, "TxPlatform"), errParam.bistatic->txPlatform);
        parsePlatform(getFirstAndOnly(bistaticXML, "RcvPlatform"), errParam.bistatic->rcvPlatform);
        mCommon.parseParameters(bistaticXML, "Parameter", errParam.bistatic->parameter);
    }
    else
    {
        throw except::Exception(Ctxt(
                "Must be one of monostatic or bistatic"));
    }
}


void CPHDXMLControl::fromXML(const XMLElem productInfoXML, ProductInfo& productInfo)
{
    XMLElem profileXML = getOptional(productInfoXML, "Profile");
    if(profileXML)
    {
        parseString(profileXML, productInfo.profile);
    }

    std::vector<XMLElem> creationInfoXML;
    productInfoXML->getElementsByTagName("CreationInfo", creationInfoXML);
    productInfo.creationInfo.resize(creationInfoXML.size());
    
    for (size_t i = 0; i < creationInfoXML.size(); ++i)
    {
        XMLElem applicationXML = getOptional(creationInfoXML[i], "Application");
        if(applicationXML)
        {
            parseString(applicationXML, productInfo.creationInfo[i].application);
        }

        parseDateTime(getFirstAndOnly(creationInfoXML[i], "DateTime"), productInfo.creationInfo[i].dateTime);

        XMLElem siteXML = getOptional(creationInfoXML[i], "Site");
        if(siteXML)
        {
            parseString(siteXML, productInfo.creationInfo[i].site);
        }
        mCommon.parseParameters(creationInfoXML[i], "Parameter", productInfo.creationInfo[i].parameter);
    }
    mCommon.parseParameters(productInfoXML, "Parameter", productInfo.parameter);

}

void CPHDXMLControl::fromXML(const XMLElem geoInfoXML, GeoInfo& geoInfo)
{

    geoInfo.setName(geoInfoXML->attribute("name"));

    mCommon.parseParameters(geoInfoXML, "Desc", geoInfo.desc);

    std::vector<XMLElem> pointXML;
    geoInfoXML->getElementsByTagName("Point", pointXML);
    geoInfo.point.resize(pointXML.size());
    for (size_t i = 0; i < pointXML.size(); ++i)
    {
        mCommon.parseLatLon(pointXML[i], geoInfo.point[i]);
    }

    //! Parse Line
    std::vector<XMLElem> lineXML;
    geoInfoXML->getElementsByTagName("Line", lineXML);
    geoInfo.line.resize(lineXML.size());
    for (size_t i = 0; i < lineXML.size(); ++i)
    {
        sscanf(lineXML[i]->attribute("size").c_str(), "%zu", &geoInfo.line[i].numEndpoints);

        std::vector<XMLElem> endpointXMLVec;
        lineXML[i]->getElementsByTagName("Endpoint", endpointXMLVec);
        geoInfo.line[i].endpoint.resize(endpointXMLVec.size());
        if(endpointXMLVec.size() != 0 && endpointXMLVec.size() < 2)
        {
            throw except::Exception(Ctxt(
                    "Line must contain atleast 2 vertices"));
        }
        for (size_t j = 0; j < endpointXMLVec.size(); ++j)
        {
            sscanf(endpointXMLVec[j]->attribute("index").c_str(), "%zu", &geoInfo.line[i].endpoint[j].index);
            mCommon.parseLatLon(endpointXMLVec[j], geoInfo.line[i].endpoint[j]);
        }

    }

    //! Parse polygon
    std::vector<XMLElem> polygonXML;
    geoInfoXML->getElementsByTagName("Polygon", polygonXML);
    geoInfo.polygon.resize(polygonXML.size());
    for (size_t i = 0; i < polygonXML.size(); ++i)
    {
        sscanf(polygonXML[i]->attribute("size").c_str(), "%zu", &geoInfo.polygon[i].numVertices);

        std::vector<XMLElem> vertexXMLVec;
        polygonXML[i]->getElementsByTagName("Vertex", vertexXMLVec);
        geoInfo.polygon[i].vertex.resize(vertexXMLVec.size());
        if(vertexXMLVec.size() != 0 && vertexXMLVec.size() < 3)
        {
            throw except::Exception(Ctxt(
                    "Polygon must contain atleast 3 vertices"));
        }
        for (size_t j = 0; j < vertexXMLVec.size(); ++j)
        {
            sscanf(vertexXMLVec[j]->attribute("index").c_str(), "%zu", &geoInfo.polygon[i].vertex[j].index);
            mCommon.parseLatLon(vertexXMLVec[j], geoInfo.polygon[i].vertex[j]);
        }

    }

    //! Parse new geoInfos
    std::vector<XMLElem> addedGeoInfoXML;
    geoInfoXML->getElementsByTagName("GeoInfo", addedGeoInfoXML);
    geoInfo.geoInfo.resize(addedGeoInfoXML.size());
    for (size_t i = 0; i < addedGeoInfoXML.size(); ++i)
    {
        // Recurses until base case: addedGeoInfoXML is empty
        fromXML(addedGeoInfoXML[i], geoInfo.geoInfo[i]);
        geoInfo.geoInfo[i].setName(addedGeoInfoXML[i]->attribute("name"));
    }
}

void CPHDXMLControl::fromXML(const XMLElem matchInfoXML, MatchInfo& matchInfo)
{
    parseUInt(getFirstAndOnly(matchInfoXML, "NumMatchTypes") , matchInfo.numMatchTypes);

    std::vector<XMLElem> matchTypeXML;
    matchInfoXML->getElementsByTagName("MatchType", matchTypeXML);
    matchInfo.matchType.resize(matchTypeXML.size());
    for (size_t i = 0; i < matchTypeXML.size(); ++i)
    {
        sscanf(matchTypeXML[i]->attribute("index").c_str(), "%zu", &matchInfo.matchType[i].index);
        parseString(getFirstAndOnly(matchTypeXML[i], "TypeID"), matchInfo.matchType[i].typeID);
        XMLElem currentIndexXML = getOptional(matchTypeXML[i], "CurrentIndex");
        if(currentIndexXML)
        {
            parseUInt(currentIndexXML, matchInfo.matchType[i].currentIndex);
        }
        parseUInt(getFirstAndOnly(matchTypeXML[i], "NumMatchCollections"), matchInfo.matchType[i].numMatchCollections);

        std::vector<XMLElem> matchCollectionXMLVec;
        matchTypeXML[i]->getElementsByTagName("MatchCollection", matchCollectionXMLVec);
        matchInfo.matchType[i].matchCollection.resize(matchCollectionXMLVec.size());
        for (size_t j = 0; j < matchCollectionXMLVec.size(); ++j)
        {
            sscanf(matchCollectionXMLVec[j]->attribute("index").c_str(), "%zu", &matchInfo.matchType[i].matchCollection[j].index);
            parseString(getFirstAndOnly(matchCollectionXMLVec[j], "CoreName"), matchInfo.matchType[i].matchCollection[j].coreName);
            XMLElem matchIndexXML = getOptional(matchCollectionXMLVec[j], "MatchIndex");
            if(matchIndexXML)
            {
                parseUInt(matchIndexXML, matchInfo.matchType[i].matchCollection[j].matchIndex);
            }
            mCommon.parseParameters(matchCollectionXMLVec[j], "Parameter", matchInfo.matchType[i].matchCollection[j].parameter);
        }
    }
}

/*
 * Creation helper functions
*/
void CPHDXMLControl::createParameterCollection(const std::string& name, six::ParameterCollection& parameterCollection,
                                        XMLElem parent) const
{
    for (size_t i = 0; i < parameterCollection.size(); ++i)
    {
        XMLElem elem = createString(name, parameterCollection[i].str(), parent);
        setAttribute(elem, "name", parameterCollection[i].getName());
    }
}

XMLElem CPHDXMLControl::createVector2D(
        const std::string& name,
        Vector2 p,
        XMLElem parent) const
{
    XMLElem e = newElement(name, getDefaultURI(), parent);
    createDouble("X", getSICommonURI(), p[0], e);
    createDouble("Y", getSICommonURI(), p[1], e);
    return e;
}

XMLElem CPHDXMLControl::createLatLonFootprint(const std::string& name,
                                                 const std::string& cornerName,
                                                 const cphd::LatLonCorners& corners,
                                                 XMLElem parent) const
{
    XMLElem footprint = newElement(name, parent);

    // Write the corners in CW order
    XMLElem vertex =
        mCommon.createLatLon(cornerName, corners.upperLeft, footprint);
    setAttribute(vertex, "index", "1");

    vertex = mCommon.createLatLon(cornerName, corners.upperRight, footprint);
    setAttribute(vertex, "index", "2");

    vertex = mCommon.createLatLon(cornerName, corners.lowerRight, footprint);
    setAttribute(vertex, "index", "3");

    vertex = mCommon.createLatLon(cornerName, corners.lowerLeft, footprint);
    setAttribute(vertex, "index", "4");

    return footprint;
}

XMLElem CPHDXMLControl::createPVPType(const std::string& name,
                            PVPType p,
                            XMLElem parent) const
{
    XMLElem pvpXML = newElement(name, parent);
    createInt("Offset", p.getOffset(), pvpXML);
    createInt("Size", p.getSize(), pvpXML);
    createString("Format", p.getFormat(), pvpXML);
    return pvpXML;
}

XMLElem CPHDXMLControl::createAPVPType(const std::string& name,
                                APVPType p,
                                XMLElem parent) const
{
    XMLElem apvpXML = newElement(name, parent);
    createString("Name", p.getName(), apvpXML);
    createInt("Offset", p.getOffset(), apvpXML);
    createInt("Size", p.getSize(), apvpXML);
    createString("Format", p.getFormat(), apvpXML);
    return apvpXML;
}

XMLElem CPHDXMLControl::createErrorParamPlatform(const std::string& name,
                            ErrorParameters::Bistatic::Platform p,
                            XMLElem parent) const
{
    XMLElem posVelErrXML = newElement("PosVelErr", parent);
    createString("Frame", p.posVelErr.frame.toString(), posVelErrXML);
    createDouble("P1", p.posVelErr.p1, posVelErrXML);
    createDouble("P2", p.posVelErr.p2, posVelErrXML);
    createDouble("P3", p.posVelErr.p3, posVelErrXML);
    createDouble("V1", p.posVelErr.v1, posVelErrXML);
    createDouble("V2", p.posVelErr.v2, posVelErrXML);
    createDouble("V3", p.posVelErr.v3, posVelErrXML);
    XMLElem corrCoefsXML = newElement("CorrCoefs", posVelErrXML);
    if(p.posVelErr.corrCoefs.get())
    {
        createDouble("P1P2", p.posVelErr.corrCoefs->p1p2, corrCoefsXML);
        createDouble("P1P3", p.posVelErr.corrCoefs->p1p3, corrCoefsXML);
        createDouble("P1V1", p.posVelErr.corrCoefs->p1v1, corrCoefsXML);
        createDouble("P1V2", p.posVelErr.corrCoefs->p1v2, corrCoefsXML);
        createDouble("P1V3", p.posVelErr.corrCoefs->p1v3, corrCoefsXML);
        createDouble("P2P3", p.posVelErr.corrCoefs->p2p3, corrCoefsXML);
        createDouble("P2V1", p.posVelErr.corrCoefs->p2v1, corrCoefsXML);
        createDouble("P2V2", p.posVelErr.corrCoefs->p2v2, corrCoefsXML);
        createDouble("P2V3", p.posVelErr.corrCoefs->p2v3, corrCoefsXML);
        createDouble("P3V1", p.posVelErr.corrCoefs->p3v1, corrCoefsXML);
        createDouble("P3V2", p.posVelErr.corrCoefs->p3v2, corrCoefsXML);
        createDouble("P3V3", p.posVelErr.corrCoefs->p3v3, corrCoefsXML);
        createDouble("V1V2", p.posVelErr.corrCoefs->v1v2, corrCoefsXML);
        createDouble("V1V3", p.posVelErr.corrCoefs->v1v3, corrCoefsXML);
        createDouble("V2V3", p.posVelErr.corrCoefs->v2v3, corrCoefsXML);
    }
    if(p.posVelErr.positionDecorr.get())
    {
        XMLElem positionDecorrXML = newElement("PositionDecorr", posVelErrXML);
        createDouble("CorrCoefZero", p.posVelErr.positionDecorr->corrCoefZero, positionDecorrXML);
        createDouble("DecorrRate", p.posVelErr.positionDecorr->decorrRate, positionDecorrXML);
    }
    return posVelErrXML;
}


/*
 * Parser helper functions
 */
void CPHDXMLControl::parseVector2D(const XMLElem vecXML, Vector2& vec) const
{
    parseDouble(getFirstAndOnly(vecXML, "X"), vec[0]);
    parseDouble(getFirstAndOnly(vecXML, "Y"), vec[1]);
}

void CPHDXMLControl::parseAreaType(const XMLElem areaXML, AreaType& area) const
{
    parseVector2D(getFirstAndOnly(areaXML, "X1Y1"), area.x1y1);
    parseVector2D(getFirstAndOnly(areaXML, "X2Y2"), area.x2y2);
    const XMLElem polygonXML = getOptional(areaXML, "Polygon");
    if (polygonXML)
    {
        std::vector<XMLElem> verticesXML;
        polygonXML->getElementsByTagName("Vertex", verticesXML);
        if (verticesXML.size() < 3)
        {
            throw except::Exception(Ctxt(
                    "Polygons must have at least 3 sides"));
        }
        area.polygon.resize(verticesXML.size());
        for (size_t ii = 0; ii < area.polygon.size(); ++ii)
        {
            Vector2& vertex = area.polygon[ii];
            const XMLElem vertexXML = verticesXML[ii];
            parseVector2D(vertexXML, vertex);
        }
    }
}

void CPHDXMLControl::parseLineSample(const XMLElem lsXML, LineSample& ls) const
{
    parseDouble(getFirstAndOnly(lsXML, "Line"), ls.line);
    parseDouble(getFirstAndOnly(lsXML, "Sample"), ls.sample);
}

void CPHDXMLControl::parseIAExtent(const XMLElem extentXML,
                                   ImageAreaXExtent& extent) const
{
    parseDouble(getFirstAndOnly(extentXML, "LineSpacing"),
                extent.lineSpacing);
    parseInt(getFirstAndOnly(extentXML, "FirstLine"),
             extent.firstLine);
    parseUInt(getFirstAndOnly(extentXML, "NumLines"),
              extent.numLines);
}

void CPHDXMLControl::parseIAExtent(const XMLElem extentXML,
                                   ImageAreaYExtent& extent) const
{
    parseDouble(getFirstAndOnly(extentXML, "SampleSpacing"),
                extent.sampleSpacing);
    parseInt(getFirstAndOnly(extentXML, "FirstSample"),
             extent.firstSample);
    parseUInt(getFirstAndOnly(extentXML, "NumSamples"),
              extent.numSamples);
}

void CPHDXMLControl::parseChannelParameters(
        const XMLElem paramXML, ChannelParameter& param) const
{
    parseString(getFirstAndOnly(paramXML, "Identifier"), param.identifier);
    parseUInt(getFirstAndOnly(paramXML, "RefVectorIndex"), param.refVectorIndex);
    parseBooleanType(getFirstAndOnly(paramXML, "FXFixed"), param.fxFixed);
    parseBooleanType(getFirstAndOnly(paramXML, "TOAFixed"), param.toaFixed);
    parseBooleanType(getFirstAndOnly(paramXML, "SRPFixed"), param.srpFixed);

    XMLElem signalXML = getOptional(paramXML, "SignalNormal");
    if (signalXML)
    {
        parseBooleanType(signalXML, param.signalNormal);
    }

    parseDouble(getFirstAndOnly(paramXML, "FxC"), param.fxC);
    parseDouble(getFirstAndOnly(paramXML, "FxBW"), param.fxBW);
    parseDouble(getOptional(paramXML, "FxBWNoise"), param.fxBWNoise);
    parseDouble(getFirstAndOnly(paramXML, "TOASaved"), param.toaSaved);

    XMLElem toaExtendedXML = getOptional(paramXML, "TOAExtended");
    if(toaExtendedXML)
    {
        param.toaExtended.reset(new TOAExtended());
        parseDouble(getFirstAndOnly(toaExtendedXML, "TOAExtSaved"), param.toaExtended->toaExtSaved);
        XMLElem lfmEclipseXML = getOptional(toaExtendedXML, "LFMEclipse");
        if(lfmEclipseXML)
        {
            param.toaExtended->lfmEclipse.reset(new TOAExtended::LFMEclipse());
            parseDouble(getFirstAndOnly(lfmEclipseXML, "FxEarlyLow"), param.toaExtended->lfmEclipse->fxEarlyLow);
            parseDouble(getFirstAndOnly(lfmEclipseXML, "FxEarlyHigh"), param.toaExtended->lfmEclipse->fxEarlyHigh);
            parseDouble(getFirstAndOnly(lfmEclipseXML, "FxLateLow"), param.toaExtended->lfmEclipse->fxLateLow);
            parseDouble(getFirstAndOnly(lfmEclipseXML, "FxLateHigh"), param.toaExtended->lfmEclipse->fxLateHigh);
        }
    }

    XMLElem dwellTimesXML = getFirstAndOnly(paramXML, "DwellTimes");
    parseString(getFirstAndOnly(dwellTimesXML, "CODId"), param.dwellTimes.codId);
    parseString(getFirstAndOnly(dwellTimesXML, "DwellId"), param.dwellTimes.dwellId);

    XMLElem imageAreaXML = getOptional(paramXML, "ImageArea");
    if(imageAreaXML)
    {
        parseAreaType(imageAreaXML, param.imageArea);
    }

    XMLElem antennaXML = getOptional(paramXML, "Antenna");
    if(antennaXML)
    {
        param.antenna.reset(new ChannelParameter::Antenna());
        parseString(getFirstAndOnly(antennaXML, "TxAPCId"), param.antenna->txAPCId);
        parseString(getFirstAndOnly(antennaXML, "TxAPATId"), param.antenna->txAPATId);
        parseString(getFirstAndOnly(antennaXML, "RcvAPCId"), param.antenna->rcvAPCId);
        parseString(getFirstAndOnly(antennaXML, "RcvAPATId"), param.antenna->rcvAPATId);
    }

    XMLElem txRcvXML = getOptional(paramXML, "TxRcv");
    if(txRcvXML)
    {
        std::vector<XMLElem> txWFIdXML;
        txRcvXML->getElementsByTagName("TxWFId", txWFIdXML);
        param.txRcv.reset(new ChannelParameter::TxRcv());
        param.txRcv->txWFId.resize(txWFIdXML.size());
        for(size_t ii = 0; ii < txWFIdXML.size(); ++ii)
        {
            parseString(txWFIdXML[ii], param.txRcv->txWFId[ii]);
        }

        std::vector<XMLElem> rcvIdXML;
        txRcvXML->getElementsByTagName("RcvId", rcvIdXML);
        param.txRcv->rcvId.resize(rcvIdXML.size());
        for(size_t ii = 0; ii < rcvIdXML.size(); ++ii)
        {
            parseString(rcvIdXML[ii], param.txRcv->rcvId[ii]);
        }
    }

    XMLElem tgtRefLevelXML = getOptional(paramXML, "TgtRefLevel");
    if(tgtRefLevelXML)
    {
        param.tgtRefLevel.reset(new TgtRefLevel());
        parseDouble(getFirstAndOnly(tgtRefLevelXML, "PTRef"), param.tgtRefLevel->ptRef);
    }

    XMLElem noiseLevelXML = getOptional(paramXML, "NoiseLevel");
    if(noiseLevelXML)
    {
        param.noiseLevel.reset(new NoiseLevel());
        parseDouble(getFirstAndOnly(noiseLevelXML, "PNRef"), param.noiseLevel->pnRef);
        parseDouble(getFirstAndOnly(noiseLevelXML, "BNRef"), param.noiseLevel->bnRef);
        if(!(param.noiseLevel->bnRef > 0 && param.noiseLevel->bnRef <= 1))
        {
            throw except::Exception(Ctxt(
                "Noise equivalent BW value must be > 0.0 and <= 1.0"));
        }

        XMLElem fxNoiseProfileXML = getOptional(noiseLevelXML, "FxNoiseProfile");
        if(fxNoiseProfileXML)
        {
            param.noiseLevel->fxNoiseProfile.reset(new FxNoiseProfile());
            std::vector<XMLElem> pointXMLVec;
            fxNoiseProfileXML->getElementsByTagName("Point", pointXMLVec);
            if(pointXMLVec.size() < 2)
            {
                throw except::Exception(Ctxt(
                    "Atleast 2 noise profile points must be provided"));
            }
            param.noiseLevel->fxNoiseProfile->point.resize(pointXMLVec.size());
            double prev_point = six::Init::undefined<double>();
            for(size_t ii = 0; ii < pointXMLVec.size(); ++ii)
            {
                double fx;
                parseDouble(getFirstAndOnly(pointXMLVec[ii], "Fx"), fx);
                parseDouble(getFirstAndOnly(pointXMLVec[ii], "PN"), param.noiseLevel->fxNoiseProfile->point[ii].pn);

                if(!six::Init::isUndefined(prev_point) && fx <= prev_point)
                {
                    throw except::Exception(Ctxt(
                        "Fx values are strictly increasing"));
                }
                param.noiseLevel->fxNoiseProfile->point[ii].fx = fx;
                prev_point = fx;
            }
        }
    }


    // Polarization
    std::vector<XMLElem> PolarizationXML;
    paramXML->getElementsByTagName("Polarization", PolarizationXML);
    for (size_t ii = 0; ii < PolarizationXML.size(); ++ii)
    {
        const XMLElem TxPolXML = getFirstAndOnly(PolarizationXML[ii], "TxPol");
        param.polarization.txPol = PolarizationType(TxPolXML->getCharacterData());
 
        const XMLElem RcvPolXML = getFirstAndOnly(PolarizationXML[ii], "RcvPol");
        param.polarization.rcvPol = PolarizationType(RcvPolXML->getCharacterData());
    }

}

void CPHDXMLControl::parsePVPType(Pvp& pvp, const XMLElem paramXML, PVPType& param) const
{
    size_t size;
    size_t offset;
    std::string format;
    parseUInt(getFirstAndOnly(paramXML, "Size"), size);
    parseUInt(getFirstAndOnly(paramXML, "Offset"), offset);
    parseString(getFirstAndOnly(paramXML, "Format"), format);
    pvp.setData(param, size, offset, format);
}

void CPHDXMLControl::parsePVPType(Pvp& pvp, const XMLElem paramXML, size_t idx) const
{
    std::string name;
    size_t size;
    size_t offset;
    std::string format;
    parseString(getFirstAndOnly(paramXML, "Name"), name);
    parseUInt(getFirstAndOnly(paramXML, "Size"), size);
    parseUInt(getFirstAndOnly(paramXML, "Offset"), offset);
    parseString(getFirstAndOnly(paramXML, "Format"), format);
    pvp.setData(size, offset, format, name, idx);
}

void CPHDXMLControl::parsePlatformParams(const XMLElem platXML, Bistatic::PlatformParams& plat) const
{
    parseDouble(getFirstAndOnly(platXML, "Time"), plat.time);
    parseDouble(getFirstAndOnly(platXML, "SlantRange"), plat.slantRange);
    parseDouble(getFirstAndOnly(platXML, "GroundRange"), plat.groundRange);
    parseDouble(getFirstAndOnly(platXML, "DopplerConeAngle"), plat.dopplerConeAngle);
    parseDouble(getFirstAndOnly(platXML, "AzimuthAngle"), plat.azimuthAngle);
    parseDouble(getFirstAndOnly(platXML, "GrazeAngle"), plat.grazeAngle);
    parseDouble(getFirstAndOnly(platXML, "IncidenceAngle"), plat.incidenceAngle);
    mCommon.parseVector3D(getFirstAndOnly(platXML, "Pos"), plat.pos);
    mCommon.parseVector3D(getFirstAndOnly(platXML, "Vel"), plat.vel);
    std::string side = "";
    parseString(getFirstAndOnly(platXML, "SideOfTrack"), side);
    plat.sideOfTrack = (side == "L" ? six::SideOfTrackType("LEFT") : six::SideOfTrackType("RIGHT"));

}

void CPHDXMLControl::parseCommon(const XMLElem imgTypeXML, ImagingType* imgType) const
{
    parseDouble(getFirstAndOnly(imgTypeXML, "TwistAngle"), imgType->twistAngle);
    parseDouble(getFirstAndOnly(imgTypeXML, "SlopeAngle"), imgType->slopeAngle);
    parseDouble(getFirstAndOnly(imgTypeXML, "LayoverAngle"), imgType->layoverAngle);
    parseDouble(getFirstAndOnly(imgTypeXML, "AzimuthAngle"), imgType->azimuthAngle);
    parseDouble(getFirstAndOnly(imgTypeXML, "GrazeAngle"), imgType->grazeAngle);
}

void CPHDXMLControl::parseDecorr(const XMLElem decorrXML, Decorr& decorr) const
{
    parseDouble(getFirstAndOnly(decorrXML, "CorrCoefZero"), decorr.corrCoefZero);
    parseDouble(getFirstAndOnly(decorrXML, "DecorrRate"), decorr.decorrRate);
}


void CPHDXMLControl::parsePosVelErr(const XMLElem posVelErrXML, PosVelErr& posVelErr) const
{
    std::string frameStr;
    parseString(getFirstAndOnly(posVelErrXML, "Frame"), frameStr);
    posVelErr.frame.mValue = scene::FrameType::fromString(frameStr);
    parseDouble(getFirstAndOnly(posVelErrXML, "P1"), posVelErr.p1);
    parseDouble(getFirstAndOnly(posVelErrXML, "P2"), posVelErr.p2);
    parseDouble(getFirstAndOnly(posVelErrXML, "P3"), posVelErr.p3);
    parseDouble(getFirstAndOnly(posVelErrXML, "V1"), posVelErr.v1);
    parseDouble(getFirstAndOnly(posVelErrXML, "V2"), posVelErr.v2);
    parseDouble(getFirstAndOnly(posVelErrXML, "V3"), posVelErr.v3);

    XMLElem corrCoefsXML = getOptional(posVelErrXML, "CorrCoefs");

    if(corrCoefsXML)
    {
        posVelErr.corrCoefs.reset(new PosVelErr::CorrCoefs());
        parseDouble(getFirstAndOnly(corrCoefsXML, "P1P2"), posVelErr.corrCoefs->p1p2);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P1P3"), posVelErr.corrCoefs->p1p3);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P1V1"), posVelErr.corrCoefs->p1v1);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P1V2"), posVelErr.corrCoefs->p1v2);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P1V3"), posVelErr.corrCoefs->p1v3);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P2P3"), posVelErr.corrCoefs->p2p3);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P2V1"), posVelErr.corrCoefs->p2v1);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P2V2"), posVelErr.corrCoefs->p2v2);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P2V3"), posVelErr.corrCoefs->p2v3);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P3V1"), posVelErr.corrCoefs->p3v1);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P3V2"), posVelErr.corrCoefs->p3v2);
        parseDouble(getFirstAndOnly(corrCoefsXML, "P3V3"), posVelErr.corrCoefs->p3v3);
        parseDouble(getFirstAndOnly(corrCoefsXML, "V1V2"), posVelErr.corrCoefs->v1v2);
        parseDouble(getFirstAndOnly(corrCoefsXML, "V1V3"), posVelErr.corrCoefs->v1v3);
        parseDouble(getFirstAndOnly(corrCoefsXML, "V2V3"), posVelErr.corrCoefs->v2v3);
    }

    XMLElem posDecorrXML = getOptional(posVelErrXML, "PositionDecorr");

    if(posDecorrXML)
    {
        posVelErr.positionDecorr.reset(new Decorr());
        parseDecorr(posDecorrXML, *(posVelErr.positionDecorr));
    }
}


void CPHDXMLControl::parsePlatform(XMLElem platXML, ErrorParameters::Bistatic::Platform& plat) const
{
    parsePosVelErr(getFirstAndOnly(platXML, "PosVelErr"), plat.posVelErr);
    XMLElem radarSensorXML = getFirstAndOnly(platXML, "RadarSensor");
    XMLElem clockFreqSFXML = getOptional(radarSensorXML, "ClockFreqSF");
    if(clockFreqSFXML)
    {
        parseDouble(clockFreqSFXML, plat.radarSensor.clockFreqSF);
    }
    parseDouble(getFirstAndOnly(radarSensorXML, "CollectionStartTime"), plat.radarSensor.collectionStartTime);
}

void CPHDXMLControl::parseSupportArrayParameter(const XMLElem paramXML, SupportArrayParameter& param, bool additionalFlag) const
{
    if(!additionalFlag)
    {
        size_t identifierVal;
        parseUInt(getFirstAndOnly(paramXML, "Identifier"), identifierVal);
        param.setIdentifier(identifierVal);
    }
    parseString(getFirstAndOnly(paramXML, "ElementFormat"), param.elementFormat);
    parseDouble(getFirstAndOnly(paramXML, "X0"), param.x0);
    parseDouble(getFirstAndOnly(paramXML, "Y0"), param.y0);
    parseDouble(getFirstAndOnly(paramXML, "XSS"), param.xSS);
    parseDouble(getFirstAndOnly(paramXML, "YSS"), param.ySS);
}

void CPHDXMLControl::parseTxRcvParameter(const XMLElem paramXML, ParameterType& param) const
{
    parseString(getFirstAndOnly(paramXML, "Identifier"), param.identifier);
    parseDouble(getFirstAndOnly(paramXML, "FreqCenter"), param.freqCenter);
    XMLElem lfmRateXML = getOptional(paramXML, "LFMRate");
    if(lfmRateXML)
    {
        parseDouble(lfmRateXML, param.lfmRate);
    }
    param.polarization = PolarizationType(getFirstAndOnly(paramXML, "Polarization")->getCharacterData());
}

}