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

// CPHD Spec is not enforced
#define ENFORCESPEC 0 // TODO: Kill?

namespace
{
typedef xml::lite::Element* XMLElem;
}

namespace cphd
{
    const char CPHDXMLControl::CPHD10_URI[] = "urn:CPHD:1.0";

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

    CPHDXMLControl::CPHDXMLControl(logging::Logger* log, bool ownLog, std::vector<std::string>& schemaPaths_in) :
        six::XMLParser(CPHD10_URI, false, log, ownLog),
        mCommon(CPHD10_URI, false, CPHD10_URI, log),
        schemaPaths(schemaPaths_in)
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




    /*
    std::string CPHDXMLControl::toXMLString(const Metadata& metadata)
    {
        std::auto_ptr<xml::lite::Document> doc(toXML(metadata));
        io::StringStream ss;
        doc->getRootElement()->print(ss);

        return (std::string("<?xml version=\"1.0\"?>") + ss.stream().str());
    }

    size_t CPHDXMLControl::getXMLsize(const Metadata& metadata)
    {
        return toXMLString(metadata).size();
    }
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

        if(!schemaPaths.empty()) {
            // Validate schema
            validate(doc, schemaPaths, log());
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
        /*
        XMLElem srpXML              = getFirstAndOnly(root, "SRP");
        XMLElem antennaXML          = getOptional(root, "Antenna");
        XMLElem vectorParametersXML = getFirstAndOnly(root, "VectorParameters");
        */

        // Parse XML for each section
        fromXML(collectionIDXML, cphd->collectionID);
        fromXML(globalXML, cphd->global);
        fromXML(sceneCoordsXML, cphd->sceneCoordinates);
        fromXML(dataXML, cphd->data);
        fromXML(channelXML, cphd->channel);
        fromXML(pvpXML, cphd->pvp);
        fromXML(dwellXML, cphd->dwell);
        fromXML(refGeoXML, cphd->referenceGeometry);
        /*
        fromXML(srpXML, cphd03->srp);

        if (antennaXML != NULL)
        {
            cphd03->antenna.reset(new Antenna());
            fromXML(antennaXML, *cphd03->antenna);
        }

        fromXML(vectorParametersXML, cphd03->vectorParameters);
        */
        return cphd;
    }

    // std::auto_ptr<Metadata> CPHDXMLControl::fromXML(const xml::lite::Document* doc,
    //               const std::vector<std::string>& schemaPaths,
    //               logging::Logger* log)
    // {
    //     // TODO: Implement and declare overload fromXML function with schema validation
    // }

    std::auto_ptr<Metadata> CPHDXMLControl::fromXML(const xml::lite::Document* doc, std::vector<std::string>& nodeNames)
    {
        std::auto_ptr<Metadata> cphd(new Metadata());

        if(!schemaPaths.empty()) {
            // Validate schema
            validate(doc, schemaPaths, log());
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
            else {
                throw except::Exception(Ctxt(
                        "Invalid node name provided"));
            }

        }
        /*
        XMLElem srpXML              = getFirstAndOnly(root, "SRP");
        XMLElem antennaXML          = getOptional(root, "Antenna");
        XMLElem vectorParametersXML = getFirstAndOnly(root, "VectorParameters");
        */

        // Parse XML for each section
        /*
        fromXML(srpXML, cphd03->srp);

        if (antennaXML != NULL)
        {
            cphd03->antenna.reset(new Antenna());
            fromXML(antennaXML, *cphd03->antenna);
        }

        fromXML(vectorParametersXML, cphd03->vectorParameters);
        */
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
                                       ImageAreaExtent& extent) const
    {
        parseDouble(getFirstAndOnly(extentXML, "LineSpacing"),
                    extent.lineSpacing);
        parseInt(getFirstAndOnly(extentXML, "FirstLine"),
                 extent.firstLine);
        parseUInt(getFirstAndOnly(extentXML, "NumLines"),
                  extent.numLines);
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
                        for (size_t ii = 0; ii < polyVertices.size(); ++ii)
                        {
                            parseLineSample(polyVertices[ii], vertices[ii]);
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
            parseDouble(getFirstAndOnly(toaExtendedXML, "TOAExtSaved"), param.toaExtended.toaExtSaved);
            XMLElem lfmEclipseXML = getOptional(toaExtendedXML, "LFMEclipse");
            if(lfmEclipseXML)
            {
                parseDouble(getFirstAndOnly(lfmEclipseXML, "FxEarlyLow"), param.toaExtended.lfmEclipse.fxEarlyLow);
                parseDouble(getFirstAndOnly(lfmEclipseXML, "FxEarlyHigh"), param.toaExtended.lfmEclipse.fxEarlyHigh);
                parseDouble(getFirstAndOnly(lfmEclipseXML, "FxLateLow"), param.toaExtended.lfmEclipse.fxLateLow);
                parseDouble(getFirstAndOnly(lfmEclipseXML, "FxLateHigh"), param.toaExtended.lfmEclipse.fxLateHigh);
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
            parseString(getFirstAndOnly(antennaXML, "TxAPCId"), param.antenna.txAPCId);
            parseString(getFirstAndOnly(antennaXML, "TxAPATId"), param.antenna.txAPATId);
            parseString(getFirstAndOnly(antennaXML, "RcvAPCId"), param.antenna.rcvAPCId);
            parseString(getFirstAndOnly(antennaXML, "RcvAPATId"), param.antenna.rcvAPATId);
        }

        XMLElem txRcvXML = getOptional(paramXML, "TxRcv");
        if(txRcvXML)
        {
            std::vector<XMLElem> txWFIdXML;
            txRcvXML->getElementsByTagName("TxWFId", txWFIdXML);
            param.txRcv.txWFId.resize(txWFIdXML.size());
            for(size_t ii = 0; ii < txWFIdXML.size(); ++ii)
            {
                parseString(txWFIdXML[ii], param.txRcv.txWFId[ii]);
            }

            std::vector<XMLElem> rcvIdXML;
            txRcvXML->getElementsByTagName("RcvId", rcvIdXML);
            param.txRcv.rcvId.resize(rcvIdXML.size());
            for(size_t ii = 0; ii < rcvIdXML.size(); ++ii)
            {
                parseString(rcvIdXML[ii], param.txRcv.rcvId[ii]);
            }
        }

        XMLElem tgtRefLevelXML = getOptional(paramXML, "TgtRefLevel");
        if(tgtRefLevelXML)
        {
            parseDouble(getFirstAndOnly(tgtRefLevelXML, "PTRef"), param.tgtRefLevel.ptRef);
        }

        // TODO: Noise Level
        XMLElem noiseLevelXML = getOptional(paramXML, "NoiseLevel");
        if(noiseLevelXML)
        {
            parseDouble(getFirstAndOnly(noiseLevelXML, "PNRef"), param.noiseLevel.pnRef);
            parseDouble(getFirstAndOnly(noiseLevelXML, "BNRef"), param.noiseLevel.bnRef);
            if(!(param.noiseLevel.bnRef > 0 && param.noiseLevel.bnRef <= 1))
            {
                throw except::Exception(Ctxt(
                    "Noise equivalent BW value must be > 0.0 and <= 1.0"));
            }

            XMLElem fxNoiseProfileXML = getOptional(noiseLevelXML, "FxNoiseProfile");
            if(fxNoiseProfileXML)
            {
                std::vector<XMLElem> pointXMLVec;
                fxNoiseProfileXML->getElementsByTagName("Point", pointXMLVec);
                if(pointXMLVec.size() < 2)
                {
                    throw except::Exception(Ctxt(
                        "Atleast 2 noise profile points must be provided"));
                }
                param.noiseLevel.fxNoiseProfile.point.resize(pointXMLVec.size());
                double prev_point = six::Init::undefined<double>();
                for(size_t ii = 0; ii < pointXMLVec.size(); ++ii)
                {
                    double fx;
                    parseDouble(getFirstAndOnly(pointXMLVec[ii], "Fx"), fx);
                    parseDouble(getFirstAndOnly(pointXMLVec[ii], "PN"), param.noiseLevel.fxNoiseProfile.point[ii].pn);

                    if(!six::Init::isUndefined(prev_point) && fx <= prev_point)
                    {
                        std::cout << "prev_point: " << prev_point << std::endl;
                        std::cout << "fx: " << fx << std::endl;

                        throw except::Exception(Ctxt(
                            "Fx values are strictly increasing"));
                    }
                    param.noiseLevel.fxNoiseProfile.point[ii].fx = fx;
                    std::cout<<"fx: "<<fx<< " loaded\n";
                    std::cout<<param.noiseLevel.fxNoiseProfile.point[ii].fx<<"\n";
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
            param.polarization.TxPol = PolarizationType(TxPolXML->getCharacterData());
     
            const XMLElem RcvPolXML = getFirstAndOnly(PolarizationXML[ii], "RcvPol");
            param.polarization.RcvPol = PolarizationType(RcvPolXML->getCharacterData());
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
            std::vector<XMLElem> addedParametersXMLVec;
            addedParametersXML->getElementsByTagName("Parameter", addedParametersXMLVec);
            channel.addedParameters.resize(addedParametersXMLVec.size());
            for(size_t ii = 0; ii < addedParametersXMLVec.size(); ++ii)
            {
                parseString(addedParametersXMLVec[ii], channel.addedParameters[ii]);
            }
        }
    }

    void CPHDXMLControl::parsePVPType(const XMLElem paramXML, PVPType& param) const {
        parseUInt(getFirstAndOnly(paramXML, "Offset"), param.offset);
        parseUInt(getFirstAndOnly(paramXML, "Size"), param.size);
        parseString(getFirstAndOnly(paramXML, "Format"), param.format);
    }

    void CPHDXMLControl::parsePVPType(const XMLElem paramXML, APVPType& param) const {
        parseString(getFirstAndOnly(paramXML, "Name"), param.name);
        parseUInt(getFirstAndOnly(paramXML, "Offset"), param.offset);
        parseUInt(getFirstAndOnly(paramXML, "Size"), param.size);
        parseString(getFirstAndOnly(paramXML, "Format"), param.format);
    }

    void CPHDXMLControl::fromXML(const XMLElem pvpXML, Pvp& pvp)
    {
        std::vector<XMLElem> pvpTypeXML;
        parsePVPType(getFirstAndOnly(pvpXML, "TxTime"), pvp.TxTime);
        parsePVPType(getFirstAndOnly(pvpXML, "TxPos"), pvp.TxPos);
        parsePVPType(getFirstAndOnly(pvpXML, "TxVel"), pvp.TxVel);
        parsePVPType(getFirstAndOnly(pvpXML, "RcvTime"), pvp.RcvTime);
        parsePVPType(getFirstAndOnly(pvpXML, "RcvPos"), pvp.RcvPos);
        parsePVPType(getFirstAndOnly(pvpXML, "RcvVel"), pvp.RcvVel);
        parsePVPType(getFirstAndOnly(pvpXML, "SRPPos"), pvp.SRPPos);
        parsePVPType(getFirstAndOnly(pvpXML, "aFDOP"), pvp.aFDOP);
        parsePVPType(getFirstAndOnly(pvpXML, "aFRR1"), pvp.aFRR1);
        parsePVPType(getFirstAndOnly(pvpXML, "aFRR2"), pvp.aFRR2);
        parsePVPType(getFirstAndOnly(pvpXML, "FX1"), pvp.FX1);
        parsePVPType(getFirstAndOnly(pvpXML, "FX2"), pvp.FX2);

        parsePVPType(getFirstAndOnly(pvpXML, "TOA1"), pvp.TOA1);
        parsePVPType(getFirstAndOnly(pvpXML, "TOA2"), pvp.TOA2);
        parsePVPType(getFirstAndOnly(pvpXML, "TDTropoSRP"), pvp.TDTropoSRP);
        parsePVPType(getFirstAndOnly(pvpXML, "SC0"), pvp.SC0);
        parsePVPType(getFirstAndOnly(pvpXML, "SCSS"), pvp.SCSS);

        const XMLElem AmpSFXML = getOptional(pvpXML, "AmpSF");
        if (AmpSFXML)
        {
            parsePVPType(AmpSFXML, pvp.AmpSF);
        }

        const XMLElem FXN1XML = getOptional(pvpXML, "FXN1");
        if (FXN1XML)
        {
            parsePVPType(FXN1XML, pvp.FX1);
        }

        const XMLElem FXN2XML = getOptional(pvpXML, "FXN2");
        if (FXN2XML)
        {
            parsePVPType(FXN2XML, pvp.FX2);
        }

        const XMLElem TOAE1XML = getOptional(pvpXML, "TOAE1");
        if (TOAE1XML)
        {
            parsePVPType(TOAE1XML, pvp.TOAE1);
        }

        const XMLElem TOAE2XML = getOptional(pvpXML, "TOAE2");
        if (TOAE2XML)
        {
            parsePVPType(TOAE2XML, pvp.TOAE2);
        }

        const XMLElem TDIonoSRPXML = getOptional(pvpXML, "TDIonoSRP");
        if (TDIonoSRPXML)
        {
            parsePVPType(TDIonoSRPXML, pvp.TDIonoSRP);
        }

        const XMLElem SIGNALXML = getOptional(pvpXML, "SIGNAL");
        if (SIGNALXML)
        {
            parsePVPType(SIGNALXML, pvp.SIGNAL);
        }

        std::vector<XMLElem> addedParamsXML;
        pvpXML->getElementsByTagName("AddedPVP", addedParamsXML);
        if(addedParamsXML.empty())
        {
            return;
        }
        pvp.AddedPVP.resize(addedParamsXML.size());
        for (size_t ii = 0; ii < addedParamsXML.size(); ++ii)
        {
            parsePVPType(addedParamsXML[ii], pvp.AddedPVP[ii]);
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
            mCommon.parsePoly2D(getFirstAndOnly(CODXML_vec[ii], "CODTimePoly"), dwell.cod[ii].CODTimePoly);
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
            mCommon.parsePoly2D(getFirstAndOnly(dtimeXML_vec[ii], "DwellTimePoly"), dwell.dtime[ii].DwellTimePoly); 
        }
    }

    void CPHDXMLControl::parsePlatform(const XMLElem platXML, Bistatic::PlatformParams& plat) const
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
            mCommon.parseVector3D(getFirstAndOnly(monoXML, "ArpPos"), refGeo.monostatic->arpPos);
            mCommon.parseVector3D(getFirstAndOnly(monoXML, "ArpVel"), refGeo.monostatic->arpVel);
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

            parsePlatform(getFirstAndOnly(biXML, "TxPlatform"), refGeo.bistatic->txPlatform);
            parsePlatform(getFirstAndOnly(biXML, "RcvPlatform"), refGeo.bistatic->rcvPlatform);
        }
        else
        {
            throw except::Exception(Ctxt(
                    "ReferenceGeometry must be exactly one of Monostatic or Bistatic element"));
        }

    }


}
