/* =========================================================================
 * This file is part of six-c++ 
 * =========================================================================
 * 
 * (C) Copyright 2004 - 2009, General Dynamics - Advanced Information Systems
 *
 * six-c++ is free software; you can redistribute it and/or modify
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
#include "six/sicd/ComplexXMLControl.h"
#include "six/sicd/ComplexData.h"
#include "six/sicd/ComplexDataBuilder.h"
#include "six/Types.h"
#include "six/Utilities.h"
#include <import/str.h>
#include <iostream>

using namespace six;
using namespace six::sicd;

typedef xml::lite::Element* XMLElem;

const char ComplexXMLControl::SICD_URI[] = "urn:SICD:0.4.0";

std::string ComplexXMLControl::getDefaultURI() const
{
    return SICD_URI;
}

std::string ComplexXMLControl::getSICommonURI() const
{
    return SICD_URI;
}

Data* ComplexXMLControl::fromXML(xml::lite::Document* doc)
{
    ComplexDataBuilder builder;
    ComplexData *sicd = builder.steal();

    XMLElem root = doc->getRootElement();

    XMLElem collectionInfoXML = getFirstAndOnly(root, "CollectionInfo");

    XMLElem imageCreationXML = NULL;

    imageCreationXML = getOptional(root, "ImageCreation");

    XMLElem imageDataXML = getFirstAndOnly(root, "ImageData");
    XMLElem geoDataXML = getFirstAndOnly(root, "GeoData");
    XMLElem gridXML = getFirstAndOnly(root, "Grid");
    XMLElem timelineXML = getFirstAndOnly(root, "Timeline");
    XMLElem positionXML = getFirstAndOnly(root, "Position");
    XMLElem radarCollectionXML = getFirstAndOnly(root, "RadarCollection");
    XMLElem imageFormationXML = getFirstAndOnly(root, "ImageFormation");
    XMLElem scpcoaXML = getFirstAndOnly(root, "SCPCOA");

    XMLElem radiometricXML = NULL;
    radiometricXML = getOptional(root, "Radiometric");

    XMLElem antennaXML = getOptional(root, "Antenna");

    XMLElem errorStatisticsXML = getOptional(root, "ErrorStatistics");

    XMLElem matchInfoXML = getOptional(root, "MatchInfo");
    XMLElem pfaXML = getOptional(root, "PFA");
    XMLElem rmaXML = getOptional(root, "RMA");

    xmlToCollectionInfo(collectionInfoXML, sicd->collectionInformation);

    if (imageCreationXML != NULL)
    {
        builder.addImageCreation();
        xmlToImageCreation(imageCreationXML, sicd->imageCreation);
    }

    xmlToImageData(imageDataXML, sicd->imageData);
    xmlToGeoData(geoDataXML, sicd->geoData);
    xmlToGrid(gridXML, sicd->grid);
    xmlToTimeline(timelineXML, sicd->timeline);
    xmlToPosition(positionXML, sicd->position);
    xmlToRadarCollection(radarCollectionXML, sicd->radarCollection);
    xmlToImageFormation(imageFormationXML, sicd->imageFormation);
    xmlToSCPCOA(scpcoaXML, sicd->scpcoa);

    if (radiometricXML != NULL)
    {
        builder.addRadiometric();
        XMLControl::fromXML(radiometricXML, sicd->radiometric);
    }

    if (antennaXML != NULL)
    {
        builder.addAntenna();
        xmlToAntenna(antennaXML, sicd->antenna);
    }

    if (errorStatisticsXML != NULL)
    {
        builder.addErrorStatistics();
        XMLControl::fromXML(errorStatisticsXML, sicd->errorStatistics);
    }

    if (matchInfoXML != NULL)
    {
        builder.addMatchInformation();
        xmlToMatchInfo(matchInfoXML, sicd->matchInformation);
    }

    if (pfaXML != NULL)
    {
        sicd->pfa = new PFA();
        xmlToPFA(pfaXML, sicd->pfa);
    }
    if (rmaXML != NULL)
    {
        sicd->rma = new RMA();
        xmlToRMA(rmaXML, sicd->rma);
    }

    return sicd;
}

xml::lite::Document* ComplexXMLControl::toXML(Data *data)
{
    if (data->getDataClass() != DataClass::COMPLEX)
    {
        throw except::Exception("Data must be SICD");
    }
    typedef xml::lite::Element* XMLElem;
    xml::lite::Document* doc = new xml::lite::Document();

    XMLElem root = newElement("SICD");
    //setAttribute(root, "xmlns", getDefaultURI());
    doc->setRootElement(root);

    ComplexData *sicd = (ComplexData*) data;

    collectionInfoToXML(sicd->collectionInformation, root);
    if (sicd->imageCreation)
    {
        imageCreationToXML(sicd->imageCreation, root);
    }
    imageDataToXML(sicd->imageData, root);
    geoDataToXML(sicd->geoData, root);
    gridToXML(sicd->grid, root);
    timelineToXML(sicd->timeline, root);
    positionToXML(sicd->position, root);
    radarCollectionToXML(sicd->radarCollection, root);
    imageFormationToXML(sicd->imageFormation, root);
    scpcoaToXML(sicd->scpcoa, root);
    if (sicd->radiometric)
        XMLControl::toXML(sicd->radiometric, root);
    if (sicd->antenna)
        antennaToXML(sicd->antenna, root);
    if (sicd->errorStatistics)
        XMLControl::toXML(sicd->errorStatistics, root);
    if (sicd->matchInformation && !sicd->matchInformation->collects.empty())
        matchInfoToXML(sicd->matchInformation, root);

    if (sicd->pfa)
        pfaToXML(sicd->pfa, root);
    else if (sicd->rma)
        rmaToXML(sicd->rma, root);
    else if (!sicd->pfa && !sicd->rma)
        throw except::Exception("One of PFA and RMA must be defined -- both "
            "are undefined.");

    //set the XMLNS
    root->setNamespacePrefix("", getDefaultURI());
    //        root->setNamespacePrefix("si", getSICommonURI());

    return doc;
}

XMLElem ComplexXMLControl::createFFTSign(std::string name, six::FFTSign sign,
                                         XMLElem parent)
{
    std::string charData = (sign == FFTSign::NEG) ? ("-1") : ("+1");
    XMLElem e = newElement(name, getDefaultURI(), charData, parent);
    xml::lite::AttributeNode node;
    node.setQName("class");
    node.setUri(getDefaultURI());
    node.setValue("xs:int");
    e->getAttributes().add(node);
    return e;
}

XMLElem ComplexXMLControl::collectionInfoToXML(CollectionInformation *collInfo,
                                               XMLElem parent)
{
    XMLElem collInfoXML = newElement("CollectionInfo", parent);

    std::string si = getSICommonURI();

    createString("CollectorName", si, collInfo->collectorName, collInfoXML);
    if (!collInfo->illuminatorName.empty())
        createString("IlluminatorName", si, collInfo->illuminatorName,
                     collInfoXML);
    createString("CoreName", si, collInfo->coreName, collInfoXML);
    if (!Init::isUndefined<CollectType>(collInfo->collectType))
        createString("CollectType", si,
                     six::toString<six::CollectType>(collInfo->collectType),
                     collInfoXML);

    XMLElem radarModeXML = newElement("RadarMode", si, collInfoXML);
    createString("ModeType", si, six::toString(collInfo->radarMode),
                 radarModeXML);
    if (!collInfo->radarModeID.empty())
        createString("ModeID", si, collInfo->radarModeID, radarModeXML);

    //TODO maybe add more class. info in the future
    createString("Classification", si, collInfo->classification.level,
                 collInfoXML);

    for (std::vector<std::string>::iterator it = collInfo->countryCodes.begin(); it
            != collInfo->countryCodes.end(); ++it)
    {
        createString("CountryCode", si, *it, collInfoXML);
    }
    addParameters("Parameter", si, collInfo->parameters, collInfoXML);
    return collInfoXML;
}

XMLElem ComplexXMLControl::imageCreationToXML(ImageCreation *imageCreation,
                                              XMLElem parent)
{
    XMLElem imageCreationXML = newElement("ImageCreation", parent);

    std::string si = getSICommonURI();

    if (!imageCreation->application.empty())
        createString("Application", si, imageCreation->application,
                     imageCreationXML);
    if (!Init::isUndefined<DateTime>(imageCreation->dateTime))
        createDateTime("DateTime", si, imageCreation->dateTime,
                       imageCreationXML);
    if (!imageCreation->site.empty())
        createString("Site", si, imageCreation->site, imageCreationXML);
    if (!imageCreation->profile.empty())
        createString("Profile", si, imageCreation->profile, imageCreationXML);
    return imageCreationXML;
}

XMLElem ComplexXMLControl::imageDataToXML(ImageData *imageData, XMLElem parent)
{
    XMLElem imageDataXML = newElement("ImageData", parent);

    createString("PixelType", six::toString(imageData->pixelType), imageDataXML);
    if (imageData->amplitudeTable)
    {
        //TODO AmpTable
    }
    createInt("NumRows", imageData->numRows, imageDataXML);
    createInt("NumCols", imageData->numCols, imageDataXML);
    createInt("FirstRow", imageData->firstRow, imageDataXML);
    createInt("FirstCol", imageData->firstCol, imageDataXML);

    createRowCol("FullImage", "NumRows", "NumCols", imageData->fullImage,
                 imageDataXML);
    createRowCol("SCPPixel", imageData->scpPixel, imageDataXML);

    //only if 3+ vertices
    unsigned int numVertices = imageData->validData.size();
    if (numVertices >= 3)
    {
        XMLElem vXML = newElement("ValidData", imageDataXML);
        setAttribute(vXML, "size", str::toString(numVertices));

        for (unsigned int i = 0; i < numVertices; ++i)
        {
            XMLElem vertexXML = createRowCol("Vertex", imageData->validData[i],
                                             vXML);
            setAttribute(vertexXML, "index", str::toString(i));
        }
    }
    return imageDataXML;
}

XMLElem ComplexXMLControl::geoDataToXML(GeoData *geoData, XMLElem parent)
{
    XMLElem geoDataXML = newElement("GeoData", parent);

    createEarthModelType("EarthModel", geoData->earthModel, geoDataXML);

    XMLElem scpXML = newElement("SCP", geoDataXML);
    createVector3D("ECF", geoData->scp.ecf, scpXML);
    createLatLonAlt("LLH", geoData->scp.llh, scpXML);

    //createFootprint
    createFootprint("ImageCorners", "ICP", geoData->imageCorners, false,
                    geoDataXML);

    //only if 3+ vertices
    unsigned int numVertices = geoData->validData.size();
    if (numVertices >= 3)
    {
        XMLElem vXML = newElement("ValidData", geoDataXML);
        setAttribute(vXML, "size", str::toString(numVertices));

        for (unsigned int i = 0; i < numVertices; ++i)
        {
            XMLElem vertexXML = createLatLon("Vertex", geoData->validData[i],
                                             vXML);
            setAttribute(vertexXML, "index", str::toString(i));
        }
    }

    for (std::vector<GeoInfo*>::iterator it = geoData->geoInfos.begin(); it
            != geoData->geoInfos.end(); ++it)
    {
        geoInfoToXML(*it, geoDataXML);
    }

    return geoDataXML;
}

XMLElem ComplexXMLControl::geoInfoToXML(GeoInfo *geoInfo, XMLElem parent)
{
    XMLElem geoInfoXML = newElement("GeoInfo", parent);
    if (!geoInfo->name.empty())
        setAttribute(geoInfoXML, "name", geoInfo->name);

    for (std::vector<GeoInfo*>::iterator it = geoInfo->geoInfos.begin(); it
            != geoInfo->geoInfos.end(); ++it)
    {
        geoInfoToXML(*it, geoInfoXML);
    }

    addParameters("Desc", geoInfo->desc, geoInfoXML);

    size_t numLatLons = geoInfo->geometryLatLon.size();
    if (numLatLons == 1)
    {
        createLatLon("Point", geoInfo->geometryLatLon[0], geoInfoXML);
    }
    else if (numLatLons >= 2)
    {
        XMLElem linePolyXML = newElement(numLatLons == 2 ? "Line" : "Polygon",
                                         geoInfoXML);

        for (int i = 0; i < numLatLons; ++i)
        {
            createLatLon(numLatLons == 2 ? "Endpoint" : "Vertex",
                         geoInfo->geometryLatLon[i], linePolyXML);
        }
    }
    return geoInfoXML;
}

XMLElem ComplexXMLControl::gridToXML(Grid *grid, XMLElem parent)
{
    XMLElem gridXML = newElement("Grid", parent);

    createString("ImagePlane", six::toString(grid->imagePlane), gridXML);
    createString("Type", six::toString(grid->type), gridXML);
    createPoly2D("TimeCOAPoly", grid->timeCOAPoly, gridXML);

    XMLElem rowDirXML = newElement("Row", gridXML);

    createVector3D("UVectECF", grid->row->unitVector, rowDirXML);
    createDouble("SS", grid->row->sampleSpacing, rowDirXML);
    createDouble("ImpRespWid", grid->row->impulseResponseWidth, rowDirXML);
    createFFTSign("Sgn", grid->row->sign, rowDirXML);
    createDouble("ImpRespBW", grid->row->impulseResponseBandwidth, rowDirXML);
    createDouble("KCtr", grid->row->kCenter, rowDirXML);
    createDouble("DeltaK1", grid->row->deltaK1, rowDirXML);
    createDouble("DeltaK2", grid->row->deltaK2, rowDirXML);

    if (grid->row->deltaKCOAPoly.orderX() >= 0
            && grid->row->deltaKCOAPoly.orderY() >= 0)
    {
        createPoly2D("DeltaKCOAPoly", grid->row->deltaKCOAPoly, rowDirXML);
    }

    if (!Init::isUndefined<std::string>(grid->row->weightType))
    {
        createString("WgtType", grid->row->weightType, rowDirXML);
    }

    if (grid->row->weights.size() > 0)
    {
        XMLElem wgtFuncXML = newElement("WgtFunc", rowDirXML);

        for (std::vector<double>::iterator it = grid->row->weights.begin(); it
                != grid->row->weights.end(); ++it)
        {
            createDouble("Wgt", *it, wgtFuncXML);
        }
    }

    XMLElem colDirXML = newElement("Col", gridXML);

    createVector3D("UVectECF", grid->col->unitVector, colDirXML);
    createDouble("SS", grid->col->sampleSpacing, colDirXML);
    createDouble("ImpRespWid", grid->col->impulseResponseWidth, colDirXML);
    createFFTSign("Sgn", grid->col->sign, colDirXML);
    createDouble("ImpRespBW", grid->col->impulseResponseBandwidth, colDirXML);
    createDouble("KCtr", grid->col->kCenter, colDirXML);
    createDouble("DeltaK1", grid->col->deltaK1, colDirXML);
    createDouble("DeltaK2", grid->col->deltaK2, colDirXML);

    if (grid->col->deltaKCOAPoly.orderX() >= 0
            && grid->col->deltaKCOAPoly.orderY() >= 0)
    {
        createPoly2D("DeltaKCOAPoly", grid->col->deltaKCOAPoly, colDirXML);
    }

    if (!Init::isUndefined<std::string>(grid->col->weightType))
    {
        createString("WgtType", grid->col->weightType, colDirXML);
    }

    if (grid->col->weights.size() > 0)
    {
        XMLElem wgtFuncXML = newElement("WgtFunc", colDirXML);

        for (std::vector<double>::iterator it = grid->col->weights.begin(); it
                != grid->col->weights.end(); ++it)
        {
            createDouble("Wgt", *it, wgtFuncXML);
        }
    }

    return gridXML;
}

XMLElem ComplexXMLControl::timelineToXML(Timeline *timeline, XMLElem parent)
{
    XMLElem timelineXML = newElement("Timeline", parent);

    createDateTime("CollectStart", timeline->collectStart, timelineXML);
    createDouble("CollectDuration", timeline->collectDuration, timelineXML);

    if (timeline->interPulsePeriod)
    {
        XMLElem ippXML = newElement("IPP", timelineXML);
        unsigned int setSize = timeline->interPulsePeriod->sets.size();
        ippXML->attribute("size") = str::toString<int>(setSize);

        for (unsigned int i = 0; i < setSize; ++i)
        {
            TimelineSet* timelineSet = timeline->interPulsePeriod->sets[i];
            XMLElem setXML = newElement("Set", ippXML);
            setXML->attribute("index") = str::toString<int>(i + 1);

            createDouble("TStart", timelineSet->tStart, setXML);
            createDouble("TEnd", timelineSet->tEnd, setXML);
            createInt("IPPStart", timelineSet->interPulsePeriodStart, setXML);
            createInt("IPPEnd", timelineSet->interPulsePeriodEnd, setXML);
            createPoly1D("IPPPoly", timelineSet->interPulsePeriodPoly, setXML);
        }
    }

    return timelineXML;
}

XMLElem ComplexXMLControl::positionToXML(Position *position, XMLElem parent)
{
    XMLElem positionXML = newElement("Position", parent);

    createPolyXYZ("ARPPoly", position->arpPoly, positionXML);
    if (position->grpPoly.order() >= 0)
        createPolyXYZ("GRPPoly", position->grpPoly, positionXML);
    if (position->txAPCPoly.order() >= 0)
        createPolyXYZ("TxAPCPoly", position->txAPCPoly, positionXML);
    if (position->rcvAPC && !position->rcvAPC->rcvAPCPolys.empty())
    {
        unsigned int numPolys = position->rcvAPC->rcvAPCPolys.size();
        XMLElem rcvXML = newElement("RcvAPC", positionXML);
        setAttribute(rcvXML, "size", str::toString(numPolys));

        for (unsigned int i = 0; i < numPolys; ++i)
        {
            PolyXYZ xyz = position->rcvAPC->rcvAPCPolys[0];
            XMLElem xyzXML = createPolyXYZ("RcvAPCPoly", xyz, rcvXML);
            setAttribute(xyzXML, "index", str::toString(i));
        }
    }
    return positionXML;
}

XMLElem ComplexXMLControl::radarCollectionToXML(RadarCollection *radar,
                                                XMLElem parent)
{
    XMLElem radarXML = newElement("RadarCollection", parent);

    if (!Init::isUndefined<int>(radar->refFrequencyIndex))
    {
        createInt("RefFreqIndex", radar->refFrequencyIndex, radarXML);
    }

    XMLElem txFreqXML = newElement("TxFrequency", radarXML);
    createDouble("Min", radar->txFrequencyMin, txFreqXML);
    createDouble("Max", radar->txFrequencyMax, txFreqXML);

    if (radar->txPolarization != PolarizationType::NOT_SET)
    {
        createString("TxPolarization", six::toString(radar->txPolarization),
                     radarXML);
    }

    if (radar->polarizationHVAnglePoly.order() >= 0)
    {
        createPoly1D("PolarizationHVAnglePoly", radar->polarizationHVAnglePoly,
                     radarXML);
    }

    if (!radar->txSequence.empty())
    {
        XMLElem txSeqXML = newElement("TxSequence", radarXML);
        for (std::vector<TxStep*>::iterator it = radar->txSequence.begin(); it
                != radar->txSequence.end(); ++it)
        {
            TxStep *tx = *it;

            XMLElem txStepXML = newElement("TxStep", txSeqXML);

            if (!Init::isUndefined<int>(tx->waveformIndex))
            {
                createInt("WFIndex", tx->waveformIndex, txStepXML);
            }
            if (tx->txPolarization != PolarizationType::NOT_SET)
            {
                createString("TxPolarization",
                             six::toString(tx->txPolarization), txStepXML);
            }
        }
    }

    if (!radar->waveform.empty())
    {
        unsigned int numWaveforms = radar->waveform.size();
        XMLElem wfXML = newElement("Waveform", radarXML);
        setAttribute(wfXML, "size", str::toString(numWaveforms));

        for (unsigned int i = 0; i < numWaveforms; ++i)
        {
            WaveformParameters *wf = radar->waveform[i];

            XMLElem wfpXML = newElement("WFParameters", wfXML);
            setAttribute(wfpXML, "index", str::toString(i));

            if (!Init::isUndefined<double>(wf->txPulseLength))
                createDouble("TxPulseLength", wf->txPulseLength, wfpXML);
            if (!Init::isUndefined<double>(wf->txRFBandwidth))
                createDouble("TxRFBandwidth", wf->txRFBandwidth, wfpXML);
            if (!Init::isUndefined<double>(wf->txFrequencyStart))
                createDouble("TxFreqStart", wf->txFrequencyStart, wfpXML);
            if (!Init::isUndefined<double>(wf->txFMRate))
                createDouble("TxFMRate", wf->txFMRate, wfpXML);
            if (wf->rcvDemodType != DemodType::NOT_SET)
                createString("RcvDemodType", six::toString(wf->rcvDemodType),
                             wfpXML);
            if (!Init::isUndefined<double>(wf->rcvWindowLength))
                createDouble("RcvWindowLength", wf->rcvWindowLength, wfpXML);
            if (!Init::isUndefined<double>(wf->adcSampleRate))
                createDouble("ADCSampleRate", wf->adcSampleRate, wfpXML);
            if (!Init::isUndefined<double>(wf->rcvIFBandwidth))
                createDouble("RcvIFBandwidth", wf->rcvIFBandwidth, wfpXML);
            if (!Init::isUndefined<double>(wf->rcvFrequencyStart))
                createDouble("RcvFreqStart", wf->rcvFrequencyStart, wfpXML);
            if (!Init::isUndefined<double>(wf->rcvFMRate))
                createDouble("RcvFMRate", wf->rcvFMRate, wfpXML);
        }
    }

    unsigned int numChannels = radar->rcvChannels.size();
    XMLElem rcvChanXML = newElement("RcvChannels", radarXML);
    setAttribute(rcvChanXML, "size", str::toString(numChannels));
    for (unsigned int i = 0; i < numChannels; ++i)
    {
        ChannelParameters *cp = radar->rcvChannels[i];
        XMLElem cpXML = newElement("ChanParameters", rcvChanXML);
        setAttribute(cpXML, "index", str::toString(i));

        if (!Init::isUndefined<int>(cp->rcvAPCIndex))
            createInt("RcvAPCIndex", cp->rcvAPCIndex, cpXML);

        if (cp->txRcvPolarization != DualPolarizationType::NOT_SET)
        {
            createString("TxRcvPolarization", six::toString<
                    DualPolarizationType>(cp->txRcvPolarization), cpXML);
        }
    }

    if (radar->area)
    {
        XMLElem areaXML = newElement("Area", radarXML);
        Area *area = radar->area;

        bool haveApcCorners = true;

        for (unsigned int i = 0; i < area->apcCorners.size(); ++i)
        {
            if (Init::isUndefined<LatLon>(area->apcCorners[i]))
            {
                haveApcCorners = false;
                break;
            }
        }

        if (haveApcCorners)
        {
            createFootprint("Corner", "APC", area->apcCorners, true, areaXML);
        }

        AreaPlane *plane = area->plane;
        if (plane)
        {
            XMLElem planeXML = newElement("Plane", areaXML);
            XMLElem refPtXML = newElement("RefPt", planeXML);

            ReferencePoint refPt = plane->referencePoint;
            if (!refPt.name.empty())
                setAttribute(refPtXML, "name", refPt.name);

            createVector3D("ECF", refPt.ecef, refPtXML);
            createDouble("Line", refPt.rowCol.row, refPtXML);
            createDouble("Sample", refPt.rowCol.col, refPtXML);

            areaLineDirectionParametersToXML("XDir", plane->xDirection,
                                             planeXML);
            areaSampleDirectionParametersToXML("YDir", plane->yDirection,
                                               planeXML);

            if (!plane->segmentList.empty())
            {
                XMLElem segListXML = newElement("SegmentList", planeXML);
                setAttribute(segListXML, "size",
                             str::toString(plane->segmentList.size()));
                for (int i = 0, size = plane->segmentList.size(); i < size; ++i)
                {
                    Segment *segment = plane->segmentList[i];
                    XMLElem segXML = newElement("Segment", segListXML);
                    setAttribute(segXML, "index", str::toString(i + 1));

                    createInt("StartLine", segment->startLine, segXML);
                    createInt("StartSample", segment->startSample, segXML);
                    createInt("EndLine", segment->endLine, segXML);
                    createInt("EndSample", segment->endSample, segXML);
                    createString("Identifier", segment->identifier, segXML);
                }
            }

            createString("Orientation",
                         six::toString<OrientationType>(plane->orientation),
                         planeXML);
        }
    }

    addParameters("Parameter", radar->parameters, radarXML);
    return radarXML;
}

XMLElem ComplexXMLControl::areaLineDirectionParametersToXML(
                                                            std::string name,
                                                            AreaDirectionParameters *adp,
                                                            XMLElem parent)
{
    XMLElem adpXML = newElement(name, parent);
    createVector3D("UVectECF", adp->unitVector, adpXML);
    createDouble("LineSpacing", adp->spacing, adpXML);
    createInt("NumLines", adp->elements, adpXML);
    createInt("FirstLine", adp->first, adpXML);
    return adpXML;
}

XMLElem ComplexXMLControl::areaSampleDirectionParametersToXML(
                                                              std::string name,
                                                              AreaDirectionParameters *adp,
                                                              XMLElem parent)
{
    XMLElem adpXML = newElement(name, parent);
    createVector3D("UVectECF", adp->unitVector, adpXML);
    createDouble("SampleSpacing", adp->spacing, adpXML);
    createInt("NumSamples", adp->elements, adpXML);
    createInt("FirstSample", adp->first, adpXML);
    return adpXML;
}

XMLElem ComplexXMLControl::imageFormationToXML(ImageFormation *imageFormation,
                                               XMLElem parent)
{
    XMLElem imageFormationXML = newElement("ImageFormation", parent);

    if (!imageFormation->segmentIdentifier.empty())
        createString("SegmentIdentifier", imageFormation->segmentIdentifier,
                     imageFormationXML);

    //TODO this is actually required by the schema, but I don't want to seg fault
    if (imageFormation->rcvChannelProcessed)
    {
        XMLElem rcvChanXML = newElement("RcvChanProc", imageFormationXML);
        createInt("NumChanProc",
                  imageFormation->rcvChannelProcessed->numChannelsProcessed,
                  rcvChanXML);
        if (!Init::isUndefined<double>(
                                       imageFormation->rcvChannelProcessed->prfScaleFactor))
            createDouble("PRFScaleFactor",
                         imageFormation->rcvChannelProcessed->prfScaleFactor,
                         rcvChanXML);

        for (std::vector<int>::iterator it =
                imageFormation->rcvChannelProcessed->channelIndex.begin(); it
                != imageFormation->rcvChannelProcessed->channelIndex.end(); ++it)
        {
            createInt("ChanIndex", *it, rcvChanXML);
        }
    }
    if (imageFormation->txRcvPolarizationProc != DualPolarizationType::NOT_SET)
    {
        createString("TxRcvPolarizationProc",
                     six::toString(imageFormation->txRcvPolarizationProc),
                     imageFormationXML);
    }

    createString("ImageFormAlgo",
                 six::toString(imageFormation->imageFormationAlgorithm),
                 imageFormationXML);

    createDouble("TStartProc", imageFormation->tStartProc, imageFormationXML);
    createDouble("TEndProc", imageFormation->tEndProc, imageFormationXML);

    XMLElem txFreqXML = newElement("TxFrequencyProc", imageFormationXML);
    createDouble("MinProc", imageFormation->txFrequencyProcMin, txFreqXML);
    createDouble("MaxProc", imageFormation->txFrequencyProcMax, txFreqXML);

    createString("STBeamComp",
                 six::toString(imageFormation->slowTimeBeamCompensation),
                 imageFormationXML);
    createString("ImageBeamComp",
                 six::toString(imageFormation->imageBeamCompensation),
                 imageFormationXML);
    createString("AzAutofocus",
                 six::toString(imageFormation->azimuthAutofocus),
                 imageFormationXML);
    createString("RgAutofocus", six::toString(imageFormation->rangeAutofocus),
                 imageFormationXML);

    for (unsigned int i = 0; i < imageFormation->processing.size(); ++i)
    {
        Processing* proc = &imageFormation->processing[i];

        XMLElem procXML = newElement("Processing", imageFormationXML);

        createString("Type", proc->type, procXML);
        require(createBooleanType("Applied", proc->applied, procXML), "Applied");
        addParameters("Parameter", proc->parameters, procXML);
    }

    if (imageFormation->polarizationCalibration)
    {
        XMLElem pcXML =
                newElement("PolarizationCalibration", imageFormationXML);

        require(
                createBooleanType(
                                  "HvAngleCompApplied",
                                  imageFormation->polarizationCalibration ->hvAngleCompensationApplied,
                                  imageFormationXML), "HvAngleCompApplied");

        require(
                createBooleanType(
                                  "DistortionCorrectionApplied",
                                  imageFormation ->polarizationCalibration->distortionCorrectionApplied,
                                  imageFormationXML),
                "DistortionCorrectionApplied");

        //TODO this is required, but doing this for safety - once we decide on a policy, maybe throw an exception
        Distortion *distortion =
                imageFormation->polarizationCalibration->distortion;
        if (distortion)
        {
            XMLElem distortionXML = newElement("Distortion", pcXML);

            //This should be optionally added...
            createDateTime("CalibrationDate", distortion->calibrationDate,
                           distortionXML);
            createDouble("A", distortion->a, distortionXML);
            createComplex("F1", distortion->f1, distortionXML);
            createComplex("Q1", distortion->q1, distortionXML);
            createComplex("Q2", distortion->q2, distortionXML);
            createComplex("F2", distortion->f2, distortionXML);
            createComplex("Q3", distortion->q3, distortionXML);
            createComplex("Q4", distortion->q4, distortionXML);

            if (!Init::isUndefined<double>(distortion->gainErrorA))
                createDouble("GainErrorA", distortion->gainErrorA,
                             distortionXML);
            if (!Init::isUndefined<double>(distortion->gainErrorF1))
                createDouble("GainErrorF1", distortion->gainErrorF1,
                             distortionXML);
            if (!Init::isUndefined<double>(distortion->gainErrorF2))
                createDouble("GainErrorF2", distortion->gainErrorF2,
                             distortionXML);
            if (!Init::isUndefined<double>(distortion->phaseErrorF1))
                createDouble("PhaseErrorF1", distortion->phaseErrorF1,
                             distortionXML);
            if (!Init::isUndefined<double>(distortion->phaseErrorF2))
                createDouble("PhaseErrorF2", distortion->phaseErrorF2,
                             distortionXML);
        }
    }
    return imageFormationXML;
}

XMLElem ComplexXMLControl::scpcoaToXML(SCPCOA *scpcoa, XMLElem parent)
{
    XMLElem scpcoaXML = newElement("SCPCOA", parent);
    createDouble("SCPTime", scpcoa->scpTime, scpcoaXML);
    createVector3D("ARPPos", scpcoa->arpPos, scpcoaXML);
    createVector3D("ARPVel", scpcoa->arpVel, scpcoaXML);
    createVector3D("ARPAcc", scpcoa->arpAcc, scpcoaXML);
    createString("SideOfTrack", six::toString(scpcoa->sideOfTrack), scpcoaXML);
    createDouble("SlantRange", scpcoa->slantRange, scpcoaXML);
    createDouble("GroundRange", scpcoa->groundRange, scpcoaXML);
    createDouble("DopplerConeAng", scpcoa->dopplerConeAngle, scpcoaXML);
    createDouble("GrazeAng", scpcoa->grazeAngle, scpcoaXML);
    createDouble("IncidenceAng", scpcoa->incidenceAngle, scpcoaXML);
    createDouble("TwistAng", scpcoa->twistAngle, scpcoaXML);
    createDouble("SlopeAng", scpcoa->slopeAngle, scpcoaXML);
    return scpcoaXML;
}

XMLElem ComplexXMLControl::antennaToXML(Antenna *antenna, XMLElem parent)
{
    XMLElem antennaXML = newElement("Antenna", parent);

    if (antenna->tx)
        antennaParametersToXML("Tx", antenna->tx, antennaXML);
    if (antenna->rcv)
        antennaParametersToXML("Rcv", antenna->rcv, antennaXML);
    if (antenna->twoWay)
        antennaParametersToXML("TwoWay", antenna->twoWay, antennaXML);

    return antennaXML;
}

XMLElem ComplexXMLControl::antennaParametersToXML(std::string name,
                                                  AntennaParameters *params,
                                                  XMLElem parent)
{
    XMLElem apXML = newElement(name, parent);

    createPolyXYZ("XAxisPoly", params->xAxisPoly, apXML);
    createPolyXYZ("YAxisPoly", params->yAxisPoly, apXML);
    createDouble("FreqZero", params->frequencyZero, apXML);

    if (params->electricalBoresight)
    {
        XMLElem ebXML = newElement("EB", apXML);
        createPoly1D("DCXPoly", params->electricalBoresight->dcxPoly, ebXML);
        createPoly1D("DCYPoly", params->electricalBoresight->dcyPoly, ebXML);
    }
    if (params->halfPowerBeamwidths)
    {
        XMLElem hpXML = newElement("HPBW", apXML);
        createDouble("DCX", params->halfPowerBeamwidths->dcx, hpXML);
        createDouble("DCY", params->halfPowerBeamwidths->dcy, hpXML);
    }
    if (params->array)
    {
        XMLElem arrXML = newElement("Array", apXML);
        createPoly2D("GainPoly", params->array->gainPoly, arrXML);
        createPoly2D("PhasePoly", params->array->phasePoly, arrXML);
    }
    if (params->element)
    {
        XMLElem elemXML = newElement("Elem", apXML);
        createPoly2D("GainPoly", params->element->gainPoly, elemXML);
        createPoly2D("PhasePoly", params->element->phasePoly, elemXML);
    }
    if (params->gainBSPoly.order() >= 0)
    {
        createPoly1D("GainBSPoly", params->gainBSPoly, apXML);
    }

    createBooleanType("EBFreqShift", params->electricalBoresightFrequencyShift,
                      apXML);
    createBooleanType("MLFreqDilation", params->mainlobeFrequencyDilation,
                      apXML);

    return apXML;
}

XMLElem ComplexXMLControl::matchInfoToXML(MatchInformation *matchInfo,
                                          XMLElem parent)
{
    XMLElem matchInfoXML = newElement("MatchInfo", parent);

    for (unsigned int i = 0; i < matchInfo->collects.size(); ++i)
    {

        MatchCollection *mc = matchInfo->collects[i];
        XMLElem mcXML = newElement("Collect", matchInfoXML);
        setAttribute(mcXML, "index", str::toString(i + 1));

        createString("CollectorName", mc->collectorName, mcXML);
        if (!mc->illuminatorName.empty())
            createString("IlluminatorName", mc->illuminatorName, mcXML);
        createString("CoreName", mc->coreName, mcXML);

        for (std::vector<std::string>::iterator it = mc->matchType.begin(); it
                != mc->matchType.end(); ++it)
        {
            createString("MatchType", *it, mcXML);
        }
        addParameters("Parameter", mc->parameters, mcXML);
    }

    return matchInfoXML;
}

XMLElem ComplexXMLControl::pfaToXML(PFA *pfa, XMLElem parent)
{
    XMLElem pfaXML = newElement("PFA", parent);

    createVector3D("FPN", pfa->focusPlaneNormal, pfaXML);
    createVector3D("IPN", pfa->imagePlaneNormal, pfaXML);
    createDouble("PolarAngRefTime", pfa->polarAngleRefTime, pfaXML);
    createPoly1D("PolarAngPoly", pfa->polarAnglePoly, pfaXML);
    createPoly1D("SpatialFreqSFPoly", pfa->spatialFrequencyScaleFactorPoly,
                 pfaXML);
    createDouble("Krg1", pfa->krg1, pfaXML);
    createDouble("Krg2", pfa->krg2, pfaXML);
    createDouble("Kaz1", pfa->kaz1, pfaXML);
    createDouble("Kaz2", pfa->kaz2, pfaXML);
    if (pfa->slowTimeDeskew)
    {
        XMLElem stdXML = newElement("STDeskew", pfaXML);
        require(createBooleanType("Applied", pfa->slowTimeDeskew->applied,
                                  stdXML), "Applied");

        createPoly2D("STDPhasePoly",
                     pfa->slowTimeDeskew->slowTimeDeskewPhasePoly, stdXML);
    }

    return pfaXML;
}

XMLElem ComplexXMLControl::rmaToXML(RMA *rma, XMLElem parent)
{
    XMLElem rmaXML = newElement("RMA", parent);

    createString("RMAlgoType", six::toString<six::RMAlgoType>(rma->algoType),
                 rmaXML);

    if (rma->rmat)
    {
        createString("ImageType", "RMAT", rmaXML);

        XMLElem rmatXML = newElement("RMAT", rmaXML);
        RMAT* rmat = rma->rmat;

        createDouble("RMRefTime", rmat->refTime, rmatXML);
        createVector3D("RMPosRef", rmat->refPos, rmatXML);
        createVector3D("RMVelRef", rmat->refVel, rmatXML);
        createPoly2D("CosDCACOAPoly", rmat->cosDCACOAPoly, rmatXML);
        createDouble("Kx1", rmat->kx1, rmatXML);
        createDouble("Kx2", rmat->kx2, rmatXML);
        createDouble("Ky1", rmat->ky1, rmatXML);
        createDouble("Ky2", rmat->ky2, rmatXML);
    }
    else if (rma->inca)
    {
        createString("ImageType", "INCA", rmaXML);

        XMLElem incaXML = newElement("INCA", rmaXML);
        INCA* inca = rma->inca;

        createPoly1D("TimeCAPoly", inca->timeCAPoly, incaXML);
        createDouble("R_CA_SCP", inca->rangeCA, incaXML);
        createDouble("FreqZero", inca->freqZero, incaXML);
        createPoly1D("DRateSFPoly", inca->dopplerRateScaleFactorPoly, incaXML);

        if (inca->dopplerCentroidPoly.orderX() >= 0
                && inca->dopplerCentroidPoly.orderY() >= 0)
            createPoly2D("DopCentroidPoly", inca->dopplerCentroidPoly, incaXML);

        if (!Init::isUndefined<BooleanType>(inca->dopplerCentroidCOA))
            createBooleanType("DopCentroidCOA", inca->dopplerCentroidCOA,
                              incaXML);
    }
    else
    {
        throw except::Exception("One of RMAT and INCA must be defined -- both "
            "are undefined.");
    }

    return rmaXML;
}

void ComplexXMLControl::xmlToCollectionInfo(XMLElem collectionInfoXML,
                                            CollectionInformation *collInfo)
{
    parseString(getFirstAndOnly(collectionInfoXML, "CollectorName"),
                collInfo->collectorName);

    XMLElem element = getOptional(collectionInfoXML, "IlluminatorName");
    if (element)
        parseString(element, collInfo->illuminatorName);

    element = getOptional(collectionInfoXML, "CoreName");
    if (element)
        parseString(element, collInfo->coreName);

    element = getOptional(collectionInfoXML, "CollectType");
    if (element)
        collInfo->collectType
                = six::toType<six::CollectType>(element->getCharacterData());

    XMLElem radarModeXML = getFirstAndOnly(collectionInfoXML, "RadarMode");

    collInfo->radarMode
            = six::toType<RadarModeType>(
                                         getFirstAndOnly(radarModeXML,
                                                         "ModeType")->getCharacterData());

    element = getOptional(radarModeXML, "ModeID");
    if (element)
        parseString(element, collInfo->radarModeID);

    parseString(getFirstAndOnly(collectionInfoXML, "Classification"),
                collInfo->classification.level);

    std::vector<XMLElem> countryCodeXML;
    collectionInfoXML->getElementsByTagName("CountryCode", countryCodeXML);

    //optional
    for (std::vector<XMLElem>::iterator it = countryCodeXML.begin(); it
            != countryCodeXML.end(); ++it)
    {
        std::string cc;

        parseString(*it, cc);
        collInfo->countryCodes.push_back(cc);
    }

    //optional
    parseParameters(collectionInfoXML, "Parameter", collInfo->parameters);
}

void ComplexXMLControl::xmlToImageCreation(XMLElem imageCreationXML,
                                           ImageCreation *imageCreation)
{
    // Optional
    XMLElem element = getOptional(imageCreationXML, "Application");
    if (element)
        parseString(element, imageCreation->application);

    element = getOptional(imageCreationXML, "DateTime");
    if (element)
        parseDateTime(element, imageCreation->dateTime);

    element = getOptional(imageCreationXML, "Site");
    if (element)
        parseString(element, imageCreation->site);

    element = getOptional(imageCreationXML, "Profile");
    if (element)
        parseString(element, imageCreation->profile);
}

void ComplexXMLControl::xmlToImageData(XMLElem imageDataXML,
                                       ImageData *imageData)
{
    imageData->pixelType
            = six::toType<PixelType>(
                                     getFirstAndOnly(imageDataXML, "PixelType")->getCharacterData());

    XMLElem ampTableXML = getOptional(imageDataXML, "AmpTable");

    if (ampTableXML != NULL)
    {
        std::vector<XMLElem> ampsXML;
        ampTableXML->getElementsByTagName("Amplitude", ampsXML);

        int i = 0;
        //TODO make sure there is at least 1 and not more than 256
        imageData->amplitudeTable = new AmplitudeTable();

        AmplitudeTable& ampTable = *(imageData->amplitudeTable);

        for (std::vector<XMLElem>::iterator it = ampsXML.begin(); it
                != ampsXML.end(); ++it)
        {
            parseDouble(*it, *(double*) ampTable[i++]);
        }
    }

    parseUInt(getFirstAndOnly(imageDataXML, "NumRows"), imageData->numRows);
    parseUInt(getFirstAndOnly(imageDataXML, "NumCols"), imageData->numCols);
    parseUInt(getFirstAndOnly(imageDataXML, "FirstRow"), imageData->firstRow);
    parseUInt(getFirstAndOnly(imageDataXML, "FirstCol"), imageData->firstCol);

    parseRowColInt(getFirstAndOnly(imageDataXML, "FullImage"), "NumRows",
                   "NumCols", imageData->fullImage);

    parseRowColInt(getFirstAndOnly(imageDataXML, "SCPPixel"), "Row", "Col",
                   imageData->scpPixel);

    XMLElem validDataXML = getOptional(imageDataXML, "ValidData");
    if (validDataXML)
    {
        std::vector<XMLElem> verticesXML;
        validDataXML->getElementsByTagName("Vertex", verticesXML);

        //TODO make sure there are at least 3
        for (std::vector<XMLElem>::iterator it = verticesXML.begin(); it
                != verticesXML.end(); ++it)
        {
            RowColInt rowCol;

            parseRowColInt(*it, "Row", "Col", rowCol);
            imageData->validData.push_back(rowCol);
        }
    }

}

void ComplexXMLControl::xmlToGeoData(XMLElem geoDataXML, GeoData *geoData)
{
    parseEarthModelType(getFirstAndOnly(geoDataXML, "EarthModel"),
                        geoData->earthModel);

    XMLElem tmpElem = getFirstAndOnly(geoDataXML, "SCP");
    parseVector3D(getFirstAndOnly(tmpElem, "ECF"), geoData->scp.ecf);
    parseLatLonAlt(getFirstAndOnly(tmpElem, "LLH"), geoData->scp.llh);

    parseFootprint(getFirstAndOnly(geoDataXML, "ImageCorners"), "ICP",
                   geoData->imageCorners, false);

    tmpElem = getOptional(geoDataXML, "ValidData");
    if (tmpElem != NULL)
    {
        parseLatLons(tmpElem, "Vertex", geoData->validData);
    }

    std::vector<XMLElem> geoInfosXML;
    geoDataXML->getElementsByTagName("GeoInfo", geoInfosXML);

    //optional
    for (std::vector<XMLElem>::iterator it = geoInfosXML.begin(); it
            != geoInfosXML.end(); ++it)
    {
        GeoInfo *gi = new GeoInfo();
        xmlToGeoInfo(*it, gi);
        geoData->geoInfos.push_back(gi);
    }

}

void ComplexXMLControl::xmlToGeoInfo(XMLElem geoInfoXML, GeoInfo* geoInfo)
{
    std::vector<XMLElem> geoInfosXML;
    geoInfoXML->getElementsByTagName("GeoInfo", geoInfosXML);
    geoInfo->name = geoInfoXML->getAttributes().getValue("name");

    int i = 0;
    //optional
    for (std::vector<XMLElem>::iterator it = geoInfosXML.begin(); it
            != geoInfosXML.end(); ++it)
    {
        GeoInfo *gi = new GeoInfo();
        xmlToGeoInfo(*it, gi);
        geoInfo->geoInfos.push_back(gi);
    }

    //optional
    parseParameters(geoInfoXML, "Desc", geoInfo->desc);

    XMLElem tmpElem = getOptional(geoInfoXML, "Point");
    if (tmpElem)
    {
        LatLon ll;
        parseLatLon(tmpElem, ll);
        geoInfo->geometryLatLon.push_back(ll);
    }
    else
    {
        std::string pointName = "Endpoint";
        tmpElem = getOptional(geoInfoXML, "Line");
        if (!tmpElem)
        {
            pointName = "Vertex";
            tmpElem = getOptional(geoInfoXML, "Polygon");
        }
        if (tmpElem)
        {
            parseLatLons(tmpElem, pointName, geoInfo->geometryLatLon);
        }
    }
}

void ComplexXMLControl::xmlToGrid(XMLElem gridXML, Grid *grid)
{
    grid->imagePlane
            = six::toType<ComplexImagePlaneType>(
                                                 getFirstAndOnly(gridXML,
                                                                 "ImagePlane")->getCharacterData());
    grid->type
            = six::toType<ComplexImageGridType>(
                                                getFirstAndOnly(gridXML, "Type")->getCharacterData());

    XMLElem tmpElem = getFirstAndOnly(gridXML, "TimeCOAPoly");
    parsePoly2D(tmpElem, grid->timeCOAPoly);

    tmpElem = getFirstAndOnly(gridXML, "Row");
    parseVector3D(getFirstAndOnly(tmpElem, "UVectECF"), grid->row->unitVector);
    parseDouble(getFirstAndOnly(tmpElem, "SS"), grid->row->sampleSpacing);
    parseDouble(getFirstAndOnly(tmpElem, "ImpRespWid"),
                grid->row->impulseResponseWidth);
    grid->row->sign
            = six::toType<six::FFTSign>(
                                        getFirstAndOnly(tmpElem, "Sgn")->getCharacterData());
    parseDouble(getFirstAndOnly(tmpElem, "ImpRespBW"),
                grid->row->impulseResponseBandwidth);
    parseDouble(getFirstAndOnly(tmpElem, "KCtr"), grid->row->kCenter);
    parseDouble(getFirstAndOnly(tmpElem, "DeltaK1"), grid->row->deltaK1);
    parseDouble(getFirstAndOnly(tmpElem, "DeltaK2"), grid->row->deltaK2);

    XMLElem optElem = getOptional(tmpElem, "DeltaKCOAPoly");
    if (optElem)
    {
        //optional
        parsePoly2D(optElem, grid->row->deltaKCOAPoly);
    }

    optElem = getOptional(tmpElem, "WgtType");
    if (optElem)
    {
        //optional
        parseString(optElem, grid->row->weightType);
    }

    XMLElem weightFuncXML = getOptional(tmpElem, "WgtFunct");
    if (weightFuncXML)
    {
        //optional
        //TODO make sure there is at least one and not more than 512 wgts
        std::vector<XMLElem> weightsXML;
        weightFuncXML->getElementsByTagName("Wgt", weightsXML);
        for (std::vector<XMLElem>::iterator it = weightsXML.begin(); it
                != weightsXML.end(); ++it)
        {
            double value;
            parseDouble(*it, value);
            grid->row->weights.push_back(value);
        }
    }

    tmpElem = getFirstAndOnly(gridXML, "Col");
    parseVector3D(getFirstAndOnly(tmpElem, "UVectECF"), grid->col->unitVector);
    parseDouble(getFirstAndOnly(tmpElem, "SS"), grid->col->sampleSpacing);
    parseDouble(getFirstAndOnly(tmpElem, "ImpRespWid"),
                grid->col->impulseResponseWidth);
    grid->col->sign
            = six::toType<six::FFTSign>(
                                        getFirstAndOnly(tmpElem, "Sgn")->getCharacterData());
    parseDouble(getFirstAndOnly(tmpElem, "ImpRespBW"),
                grid->col->impulseResponseBandwidth);
    parseDouble(getFirstAndOnly(tmpElem, "KCtr"), grid->col->kCenter);
    parseDouble(getFirstAndOnly(tmpElem, "DeltaK1"), grid->col->deltaK1);
    parseDouble(getFirstAndOnly(tmpElem, "DeltaK2"), grid->col->deltaK2);

    optElem = getOptional(tmpElem, "DeltaKCOAPoly");
    if (optElem)
    {
        //optional
        parsePoly2D(optElem, grid->col->deltaKCOAPoly);
    }

    optElem = getOptional(tmpElem, "WgtType");
    if (optElem)
    {
        //optional
        parseString(optElem, grid->col->weightType);
    }

    weightFuncXML = getOptional(tmpElem, "WgtFunct");
    if (weightFuncXML)
    {
        //optional

        //TODO make sure there is at least one and not more than 512 wgts
        std::vector<XMLElem> weightsXML;
        weightFuncXML->getElementsByTagName("Wgt", weightsXML);
        for (std::vector<XMLElem>::iterator it = weightsXML.begin(); it
                != weightsXML.end(); ++it)
        {
            double value;
            parseDouble(*it, value);
            grid->col->weights.push_back(value);
        }
    }
}

void ComplexXMLControl::xmlToTimeline(XMLElem timelineXML, Timeline *timeline)
{
    parseDateTime(getFirstAndOnly(timelineXML, "CollectStart"),
                  timeline->collectStart);
    parseDouble(getFirstAndOnly(timelineXML, "CollectDuration"),
                timeline->collectDuration);

    XMLElem ippXML = getOptional(timelineXML, "IPP");
    if (ippXML)
    {
        timeline->interPulsePeriod = new InterPulsePeriod();
        //TODO make sure there is at least one
        std::vector<XMLElem> setsXML;
        ippXML->getElementsByTagName("Set", setsXML);
        for (std::vector<XMLElem>::iterator it = setsXML.begin(); it
                != setsXML.end(); ++it)
        {
            // Use the first set that is already available.
            TimelineSet* ts = new TimelineSet();
            parseDouble(getFirstAndOnly(*it, "TStart"), ts->tStart);
            parseDouble(getFirstAndOnly(*it, "TEnd"), ts->tEnd);
            parseInt(getFirstAndOnly(*it, "IPPStart"),
                     ts->interPulsePeriodStart);
            parseInt(getFirstAndOnly(*it, "IPPEnd"), ts->interPulsePeriodEnd);
            parsePoly1D(getFirstAndOnly(*it, "IPPPoly"),
                        ts->interPulsePeriodPoly);
            timeline->interPulsePeriod->sets.push_back(ts);
        }

        // Required to have at least one timeline set.
        if (timeline->interPulsePeriod->sets.size() == 0)
            timeline->interPulsePeriod->sets.push_back(new TimelineSet());
    }
}

void ComplexXMLControl::xmlToPosition(XMLElem positionXML, Position *position)
{
    XMLElem tmpElem = getFirstAndOnly(positionXML, "ARPPoly");
    parsePolyXYZ(tmpElem, position->arpPoly);

    tmpElem = getOptional(positionXML, "GRPPoly");
    if (tmpElem)
    {
        //optional
        parsePolyXYZ(tmpElem, position->grpPoly);
    }

    tmpElem = getOptional(positionXML, "TxAPCPoly");
    if (tmpElem)
    {
        //optional
        parsePolyXYZ(tmpElem, position->txAPCPoly);
    }

    tmpElem = getOptional(positionXML, "RcvAPC");
    if (tmpElem)
    {
        //optional
        position->rcvAPC = new RcvAPC();

        //TODO make sure there is at least one
        std::vector<XMLElem> polysXML;
        tmpElem->getElementsByTagName("RcvAPCPoly", polysXML);
        for (std::vector<XMLElem>::iterator it = polysXML.begin(); it
                != polysXML.end(); ++it)
        {
            PolyXYZ p;
            parsePolyXYZ(*it, p);
            position->rcvAPC->rcvAPCPolys.push_back(p);
        }
    }
}

void ComplexXMLControl::xmlToRadarCollection(XMLElem radarCollectionXML,
                                             RadarCollection *radarCollection)
{
    XMLElem tmpElem = NULL;
    XMLElem optElem = NULL;

    tmpElem = getOptional(radarCollectionXML, "RefFreqIndex");
    if (tmpElem)
    {
        //optional
        parseInt(tmpElem, radarCollection->refFrequencyIndex);
    }

    tmpElem = getFirstAndOnly(radarCollectionXML, "TxFrequency");
    parseDouble(getFirstAndOnly(tmpElem, "Min"),
                radarCollection->txFrequencyMin);
    parseDouble(getFirstAndOnly(tmpElem, "Max"),
                radarCollection->txFrequencyMax);

    tmpElem = getOptional(radarCollectionXML, "TxPolarization");
    if (tmpElem)
    {
        //optional
        radarCollection->txPolarization
                = six::toType<PolarizationType>(tmpElem->getCharacterData());
    }

    tmpElem = getOptional(radarCollectionXML, "PolarizationHVAnglePoly");
    if (tmpElem)
    {
        //optional
        parsePoly1D(tmpElem, radarCollection->polarizationHVAnglePoly);
    }

    tmpElem = getOptional(radarCollectionXML, "TxSequence");
    if (tmpElem)
    {
        //optional
        //TODO make sure there is at least one
        std::vector<XMLElem> txStepsXML;
        tmpElem->getElementsByTagName("TxStep", txStepsXML);
        for (std::vector<XMLElem>::iterator it = txStepsXML.begin(); it
                != txStepsXML.end(); ++it)
        {
            TxStep* step = new TxStep();

            optElem = getOptional(*it, "WFIndex");
            if (optElem)
            {
                //optional
                parseInt(optElem, step->waveformIndex);
            }

            optElem = getOptional(*it, "TxPolarization");
            if (optElem)
            {
                //optional
                step->txPolarization
                        = six::toType<PolarizationType>(
                                                        optElem->getCharacterData());
            }

            radarCollection->txSequence.push_back(step);
        }
    }

    tmpElem = getOptional(radarCollectionXML, "Waveform");
    if (tmpElem)
    {
        //optional
        //TODO make sure there is at least one
        std::vector<XMLElem> wfParamsXML;
        tmpElem->getElementsByTagName("WFParameters", wfParamsXML);
        for (std::vector<XMLElem>::iterator it = wfParamsXML.begin(); it
                != wfParamsXML.end(); ++it)
        {
            WaveformParameters* wfParams = new WaveformParameters();

            optElem = getOptional(*it, "TxPulseLength");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->txPulseLength);
            }

            optElem = getOptional(*it, "TxRFBandwidth");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->txRFBandwidth);
            }

            optElem = getOptional(*it, "TxFreqStart");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->txFrequencyStart);
            }

            optElem = getOptional(*it, "TxFMRate");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->txFMRate);
            }

            optElem = getOptional(*it, "RcvDemodType");
            if (optElem)
            {
                //optional
                wfParams->rcvDemodType
                        = six::toType<DemodType>(optElem->getCharacterData());
            }

            optElem = getOptional(*it, "RcvWindowLength");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->rcvWindowLength);
            }

            optElem = getOptional(*it, "ADCSampleRate");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->adcSampleRate);
            }

            optElem = getOptional(*it, "RcvIFBandwidth");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->rcvIFBandwidth);
            }

            optElem = getOptional(*it, "RcvFreqStart");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->rcvFrequencyStart);
            }

            optElem = getOptional(*it, "RcvFMRate");
            if (optElem)
            {
                //optional
                parseDouble(optElem, wfParams->rcvFMRate);
            }

            radarCollection->waveform.push_back(wfParams);
        }
    }

    tmpElem = getFirstAndOnly(radarCollectionXML, "RcvChannels");

    //optional
    std::vector<XMLElem> channelsXML;
    tmpElem->getElementsByTagName("ChanParameters", channelsXML);
    for (std::vector<XMLElem>::iterator it = channelsXML.begin(); it
            != channelsXML.end(); ++it)
    {
        ChannelParameters* chanParams = new ChannelParameters();

        XMLElem childXML = getOptional(*it, "RcvAPCIndex");
        if (childXML)
        {
            parseInt(childXML, chanParams->rcvAPCIndex);
        }

        childXML = getOptional(*it, "TxRcvPolarization");
        if (childXML)
        {
            //optional
            chanParams->txRcvPolarization
                    = six::toType<DualPolarizationType>(
                                                        childXML->getCharacterData());
        }

        radarCollection->rcvChannels.push_back(chanParams);
    }

    XMLElem areaXML = getOptional(radarCollectionXML, "Area");
    if (areaXML)
    {
        //optional
        radarCollection->area = new Area();

        optElem = getOptional(areaXML, "Corner");
        if (optElem)
        {
            //optional
            parseFootprint(optElem, "APC", radarCollection->area->apcCorners,
                           true);
        }

        XMLElem planeXML = getOptional(areaXML, "Plane");
        if (planeXML)
        {
            //optional
            radarCollection->area->plane = new AreaPlane();

            XMLElem refPtXML = getFirstAndOnly(planeXML, "RefPt");
            try
            {
                radarCollection->area->plane->referencePoint.name
                        = refPtXML->getAttributes().getValue("name");
            }
            catch (except::Exception &ex)
            {
            }

            parseVector3D(getFirstAndOnly(refPtXML, "ECF"),
                          radarCollection->area->plane->referencePoint.ecef);
            parseDouble(getFirstAndOnly(refPtXML, "Line"),
                        radarCollection->area->plane->referencePoint.rowCol.row);
            parseDouble(getFirstAndOnly(refPtXML, "Sample"),
                        radarCollection->area->plane->referencePoint.rowCol.col);

            XMLElem dirXML = getFirstAndOnly(planeXML, "XDir");
            parseVector3D(getFirstAndOnly(dirXML, "UVectECF"),
                          radarCollection->area->plane->xDirection->unitVector);
            parseDouble(getFirstAndOnly(dirXML, "LineSpacing"),
                        radarCollection->area->plane->xDirection->spacing);
            parseUInt(getFirstAndOnly(dirXML, "NumLines"),
                      radarCollection->area->plane->xDirection->elements);
            parseUInt(getFirstAndOnly(dirXML, "FirstLine"),
                      radarCollection->area->plane->xDirection->first);

            dirXML = getFirstAndOnly(planeXML, "YDir");
            parseVector3D(getFirstAndOnly(dirXML, "UVectECF"),
                          radarCollection->area->plane->yDirection->unitVector);
            parseDouble(getFirstAndOnly(dirXML, "SampleSpacing"),
                        radarCollection->area->plane->yDirection->spacing);
            parseUInt(getFirstAndOnly(dirXML, "NumSamples"),
                      radarCollection->area->plane->yDirection->elements);
            parseUInt(getFirstAndOnly(dirXML, "FirstSample"),
                      radarCollection->area->plane->yDirection->first);

            XMLElem segmentListXML = getOptional(planeXML, "SegmentList");
            if (segmentListXML != NULL)
            {
                //TODO make sure there is at least one
                std::vector<XMLElem> segmentsXML;
                segmentListXML->getElementsByTagName("Segment", segmentsXML);

                for (std::vector<XMLElem>::iterator it = segmentsXML.begin(); it
                        != segmentsXML.end(); ++it)
                {
                    Segment* seg = new Segment();

                    parseInt(getFirstAndOnly(*it, "StartLine"), seg->startLine);
                    parseInt(getFirstAndOnly(*it, "StartSample"),
                             seg->startSample);
                    parseInt(getFirstAndOnly(*it, "EndLine"), seg->endLine);
                    parseInt(getFirstAndOnly(*it, "EndSample"), seg->endSample);
                    parseString(getFirstAndOnly(*it, "Identifier"),
                                seg->identifier);

                    radarCollection->area->plane->segmentList.push_back(seg);
                }
            }

            radarCollection->area->plane->orientation
                    = six::toType<OrientationType>(
                                                   getFirstAndOnly(planeXML,
                                                                   "Orientation") ->getCharacterData());
        }
    }

    parseParameters(radarCollectionXML, "Parameter",
                    radarCollection->parameters);
}

void ComplexXMLControl::xmlToImageFormation(XMLElem imageFormationXML,
                                            ImageFormation *imageFormation)
{
    XMLElem tmpElem = getOptional(imageFormationXML, "SegmentIdentifier");
    if (tmpElem)
    {
        //optional
        parseString(tmpElem, imageFormation->segmentIdentifier);
    }

    tmpElem = getFirstAndOnly(imageFormationXML, "RcvChanProc");

    parseUInt(getFirstAndOnly(tmpElem, "NumChanProc"),
              imageFormation->rcvChannelProcessed->numChannelsProcessed);

    XMLElem prfXML = getOptional(tmpElem, "PRFScaleFactor");
    if (prfXML)
    {
        //optional
        parseDouble(prfXML, imageFormation->rcvChannelProcessed->prfScaleFactor);
    }

    //TODO make sure there is at least one
    std::vector<XMLElem> chansXML;
    tmpElem->getElementsByTagName("ChanIndex", chansXML);
    for (std::vector<XMLElem>::iterator it = chansXML.begin(); it
            != chansXML.end(); ++it)
    {
        int value;
        parseInt(*it, value);
        imageFormation->rcvChannelProcessed->channelIndex.push_back(value);
    }

    tmpElem = getOptional(imageFormationXML, "TxRcvPolarizationProc");

    if (tmpElem)
    {
        imageFormation->txRcvPolarizationProc = six::toType<
                DualPolarizationType>(tmpElem->getCharacterData());
    }

    imageFormation->imageFormationAlgorithm
            = six::toType<ImageFormationType>(
                                              getFirstAndOnly(
                                                              imageFormationXML,
                                                              "ImageFormAlgo")->getCharacterData());

    parseDouble(getFirstAndOnly(imageFormationXML, "TStartProc"),
                imageFormation->tStartProc);

    parseDouble(getFirstAndOnly(imageFormationXML, "TEndProc"),
                imageFormation->tEndProc);

    tmpElem = getFirstAndOnly(imageFormationXML, "TxFrequencyProc");

    parseDouble(getFirstAndOnly(tmpElem, "MinProc"),
                imageFormation->txFrequencyProcMin);

    parseDouble(getFirstAndOnly(tmpElem, "MaxProc"),
                imageFormation->txFrequencyProcMax);

    imageFormation->slowTimeBeamCompensation
            = six::toType<SlowTimeBeamCompensationType>(
                                                        getFirstAndOnly(
                                                                        imageFormationXML,
                                                                        "STBeamComp")->getCharacterData());

    imageFormation->imageBeamCompensation
            = six::toType<ImageBeamCompensationType>(
                                                     getFirstAndOnly(
                                                                     imageFormationXML,
                                                                     "ImageBeamComp")->getCharacterData());

    imageFormation->azimuthAutofocus
            = six::toType<AutofocusType>(
                                         getFirstAndOnly(imageFormationXML,
                                                         "AzAutofocus")->getCharacterData());

    imageFormation->rangeAutofocus
            = six::toType<AutofocusType>(
                                         getFirstAndOnly(imageFormationXML,
                                                         "RgAutofocus")->getCharacterData());

    std::vector<XMLElem> procXML;
    imageFormationXML->getElementsByTagName("Processing", procXML);

    for (unsigned int i = 0; i < procXML.size(); ++i)
    {
        Processing* proc = new Processing();

        parseString(getFirstAndOnly(procXML[i], "Type"), proc->type);
        parseBooleanType(getFirstAndOnly(procXML[i], "Applied"), proc->applied);
        parseParameters(procXML[i], "Parameter", proc->parameters);

        imageFormation->processing.push_back(*proc);
    }

    XMLElem polCalXML = getOptional(imageFormationXML,
                                    "PolarizationCalibration");
    if (polCalXML)
    {
        //optional
        imageFormation->polarizationCalibration = new PolarizationCalibration();
        imageFormation->polarizationCalibration->distortion = new Distortion();

        parseBooleanType(
                         getFirstAndOnly(polCalXML, "HVAngleCompApplied"),
                         imageFormation->polarizationCalibration ->hvAngleCompensationApplied);

        parseBooleanType(
                         getFirstAndOnly(polCalXML,
                                         "DistortionCorrectionApplied"),
                         imageFormation->polarizationCalibration ->distortionCorrectionApplied);

        XMLElem distortionXML = getFirstAndOnly(polCalXML, "Distortion");

        XMLElem calibDateXML = getOptional(distortionXML, "CalibrationDate");
        if (calibDateXML)
        {
            parseDateTime(
                          calibDateXML,
                          imageFormation->polarizationCalibration ->distortion->calibrationDate);
        }

        parseDouble(getFirstAndOnly(distortionXML, "A"),
                    imageFormation->polarizationCalibration->distortion->a);

        parseComplex(getFirstAndOnly(distortionXML, "F1"),
                     imageFormation->polarizationCalibration->distortion->f1);
        parseComplex(getFirstAndOnly(distortionXML, "Q1"),
                     imageFormation->polarizationCalibration->distortion->q1);
        parseComplex(getFirstAndOnly(distortionXML, "Q2"),
                     imageFormation->polarizationCalibration->distortion->q2);
        parseComplex(getFirstAndOnly(distortionXML, "F2"),
                     imageFormation->polarizationCalibration->distortion->f2);
        parseComplex(getFirstAndOnly(distortionXML, "Q3"),
                     imageFormation->polarizationCalibration->distortion->q3);
        parseComplex(getFirstAndOnly(distortionXML, "Q4"),
                     imageFormation->polarizationCalibration->distortion->q4);

        XMLElem gainErrorXML = getOptional(distortionXML, "GainErrorA");
        if (gainErrorXML)
        {
            parseDouble(
                        gainErrorXML,
                        imageFormation->polarizationCalibration ->distortion->gainErrorA);
        }

        gainErrorXML = getOptional(distortionXML, "GainErrorF1");
        if (gainErrorXML)
        {
            parseDouble(
                        gainErrorXML,
                        imageFormation->polarizationCalibration ->distortion->gainErrorF1);
        }

        gainErrorXML = getOptional(distortionXML, "GainErrorF2");
        if (gainErrorXML)
        {
            parseDouble(
                        gainErrorXML,
                        imageFormation->polarizationCalibration ->distortion->gainErrorF2);
        }

        XMLElem phaseErrorXML = getOptional(distortionXML, "PhaseErrorF1");
        if (phaseErrorXML)
        {
            parseDouble(
                        phaseErrorXML,
                        imageFormation->polarizationCalibration ->distortion->phaseErrorF1);
        }

        phaseErrorXML = getOptional(distortionXML, "PhaseErrorF2");
        if (phaseErrorXML)
        {
            parseDouble(
                        phaseErrorXML,
                        imageFormation->polarizationCalibration ->distortion->phaseErrorF2);
        }

    }
}

void ComplexXMLControl::xmlToSCPCOA(XMLElem scpcoaXML, SCPCOA *scpcoa)
{
    parseDouble(getFirstAndOnly(scpcoaXML, "SCPTime"), scpcoa->scpTime);

    parseVector3D(getFirstAndOnly(scpcoaXML, "ARPPos"), scpcoa->arpPos);
    parseVector3D(getFirstAndOnly(scpcoaXML, "ARPVel"), scpcoa->arpVel);
    parseVector3D(getFirstAndOnly(scpcoaXML, "ARPAcc"), scpcoa->arpAcc);

    parseSideOfTrackType(getFirstAndOnly(scpcoaXML, "SideOfTrack"),
                         scpcoa->sideOfTrack);
    parseDouble(getFirstAndOnly(scpcoaXML, "SlantRange"), scpcoa->slantRange);
    parseDouble(getFirstAndOnly(scpcoaXML, "GroundRange"), scpcoa->groundRange);
    parseDouble(getFirstAndOnly(scpcoaXML, "DopplerConeAng"),
                scpcoa->dopplerConeAngle);
    parseDouble(getFirstAndOnly(scpcoaXML, "GrazeAng"), scpcoa->grazeAngle);
    parseDouble(getFirstAndOnly(scpcoaXML, "IncidenceAng"),
                scpcoa->incidenceAngle);
    parseDouble(getFirstAndOnly(scpcoaXML, "TwistAng"), scpcoa->twistAngle);
    parseDouble(getFirstAndOnly(scpcoaXML, "SlopeAng"), scpcoa->slopeAngle);
}

void ComplexXMLControl::xmlToAntennaParams(XMLElem antennaParamsXML,
                                           AntennaParameters* params)
{
    parsePolyXYZ(getFirstAndOnly(antennaParamsXML, "XAxisPoly"),
                 params->xAxisPoly);
    parsePolyXYZ(getFirstAndOnly(antennaParamsXML, "YAxisPoly"),
                 params->yAxisPoly);
    parseDouble(getFirstAndOnly(antennaParamsXML, "FreqZero"),
                params->frequencyZero);

    XMLElem tmpElem = getOptional(antennaParamsXML, "EB");
    if (tmpElem)
    {
        params->electricalBoresight = new ElectricalBoresight();
        parsePoly1D(getFirstAndOnly(tmpElem, "DCXPoly"),
                    params->electricalBoresight->dcxPoly);
        parsePoly1D(getFirstAndOnly(tmpElem, "DCYPoly"),
                    params->electricalBoresight->dcyPoly);
    }

    tmpElem = getOptional(antennaParamsXML, "HPBW");
    if (tmpElem)
    {
        params->halfPowerBeamwidths = new HalfPowerBeamwidths();
        parseDouble(getFirstAndOnly(tmpElem, "DCX"),
                    params->halfPowerBeamwidths->dcx);
        parseDouble(getFirstAndOnly(tmpElem, "DCY"),
                    params->halfPowerBeamwidths->dcy);
    }

    tmpElem = getOptional(antennaParamsXML, "Array");
    if (tmpElem)
    {
        params->array = new GainAndPhasePolys();
        parsePoly2D(getFirstAndOnly(tmpElem, "GainPoly"),
                    params->array->gainPoly);
        parsePoly2D(getFirstAndOnly(tmpElem, "PhasePoly"),
                    params->array->phasePoly);
    }

    tmpElem = getOptional(antennaParamsXML, "Elem");
    if (tmpElem)
    {
        params->element = new GainAndPhasePolys();
        parsePoly2D(getFirstAndOnly(tmpElem, "GainPoly"),
                    params->element->gainPoly);
        parsePoly2D(getFirstAndOnly(tmpElem, "PhasePoly"),
                    params->element->phasePoly);
    }

    tmpElem = getOptional(antennaParamsXML, "GainBSPoly");
    if (tmpElem)
    {
        //optional
        parsePoly1D(tmpElem, params->gainBSPoly);
    }

    tmpElem = getOptional(antennaParamsXML, "EBFreqShift");
    if (tmpElem)
    {
        //optional
        parseBooleanType(tmpElem, params->electricalBoresightFrequencyShift);
    }

    tmpElem = getOptional(antennaParamsXML, "MLFreqDilation");
    if (tmpElem)
    {
        //optional
        parseBooleanType(tmpElem, params->mainlobeFrequencyDilation);
    }
}

void ComplexXMLControl::xmlToAntenna(XMLElem antennaXML, Antenna *antenna)
{
    XMLElem antennaParamsXML = getOptional(antennaXML, "Tx");
    if (antennaParamsXML)
    {
        antenna->tx = new AntennaParameters();
        xmlToAntennaParams(antennaParamsXML, antenna->tx);
    }

    antennaParamsXML = getOptional(antennaXML, "Rcv");
    if (antennaParamsXML)
    {
        antenna->rcv = new AntennaParameters();
        xmlToAntennaParams(antennaParamsXML, antenna->rcv);
    }

    antennaParamsXML = getOptional(antennaXML, "TwoWay");
    if (antennaParamsXML)
    {
        antenna->twoWay = new AntennaParameters();
        xmlToAntennaParams(antennaParamsXML, antenna->twoWay);
    }
}

void ComplexXMLControl::xmlToMatchInfo(XMLElem matchInfoXML,
                                       MatchInformation *matchInfo)
{
    XMLElem optElem = NULL;

    //TODO make sure there is at least one
    std::vector<XMLElem> collectsXML;
    matchInfoXML->getElementsByTagName("Collect", collectsXML);
    for (std::vector<XMLElem>::iterator it = collectsXML.begin(); it
            != collectsXML.end(); ++it)
    {
        MatchCollection* coll = new MatchCollection();

        parseString(getFirstAndOnly(*it, "CollectorName"), coll->collectorName);

        optElem = getOptional(*it, "IlluminatorName");
        if (optElem)
        {
            //optional
            parseString(optElem, coll->illuminatorName);
        }

        parseString(getFirstAndOnly(*it, "CoreName"), coll->coreName);

        //optional
        std::vector<XMLElem> matchTypesXML;
        (*it)->getElementsByTagName("MatchType", matchTypesXML);
        for (std::vector<XMLElem>::iterator it2 = collectsXML.begin(); it2
                != collectsXML.end(); ++it2)
        {
            std::string value;

            parseString(*it2, value);
            coll->matchType.push_back(value);
        }

        //optional
        parseParameters(*it, "Parameter", coll->parameters);

        matchInfo->collects.push_back(coll);
    }
}

void ComplexXMLControl::xmlToPFA(XMLElem pfaXML, PFA *pfa)
{
    parseVector3D(getFirstAndOnly(pfaXML, "FPN"), pfa->focusPlaneNormal);
    parseVector3D(getFirstAndOnly(pfaXML, "IPN"), pfa->imagePlaneNormal);
    parseDouble(getFirstAndOnly(pfaXML, "PolarAngRefTime"),
                pfa->polarAngleRefTime);
    parsePoly1D(getFirstAndOnly(pfaXML, "PolarAngPoly"), pfa->polarAnglePoly);
    parsePoly1D(getFirstAndOnly(pfaXML, "SpatialFreqSFPoly"),
                pfa->spatialFrequencyScaleFactorPoly);
    parseDouble(getFirstAndOnly(pfaXML, "Krg1"), pfa->krg1);
    parseDouble(getFirstAndOnly(pfaXML, "Krg2"), pfa->krg2);
    parseDouble(getFirstAndOnly(pfaXML, "Kaz1"), pfa->kaz1);
    parseDouble(getFirstAndOnly(pfaXML, "Kaz2"), pfa->kaz2);

    XMLElem deskewXML = getOptional(pfaXML, "STDeskew");
    if (deskewXML)
    {
        pfa->slowTimeDeskew = new SlowTimeDeskew();
        parseBooleanType(getFirstAndOnly(deskewXML, "Applied"),
                         pfa->slowTimeDeskew->applied);

        parsePoly2D(getFirstAndOnly(deskewXML, "STDSPhasePoly"),
                    pfa->slowTimeDeskew->slowTimeDeskewPhasePoly);
    }
}

void ComplexXMLControl::xmlToRMA(XMLElem rmaXML, RMA* rma)
{
    rma->algoType
            = six::toType<RMAlgoType>(
                                      getFirstAndOnly(rmaXML, "RMAlgoType")->getCharacterData());

    XMLElem rmatElem = getOptional(rmaXML, "RMAT");

    if (rmatElem)
    {
        rma->rmat = new RMAT();
        RMAT* rmat = rma->rmat;

        parseDouble(getFirstAndOnly(rmatElem, "RMRefTime"), rmat->refTime);
        parseVector3D(getFirstAndOnly(rmatElem, "RMPosRef"), rmat->refPos);
        parseVector3D(getFirstAndOnly(rmatElem, "RMVelRef"), rmat->refVel);
        parsePoly2D(getFirstAndOnly(rmatElem, "CosDCACOAPoly"),
                    rmat->cosDCACOAPoly);
        parseDouble(getFirstAndOnly(rmatElem, "Kx1"), rmat->kx1);
        parseDouble(getFirstAndOnly(rmatElem, "Kx2"), rmat->kx2);
        parseDouble(getFirstAndOnly(rmatElem, "Ky1"), rmat->ky1);
        parseDouble(getFirstAndOnly(rmatElem, "Ky2"), rmat->ky2);
    }

    XMLElem incaElem = getOptional(rmaXML, "INCA");

    if (incaElem)
    {
        rma->inca = new INCA();
        INCA* inca = rma->inca;

        parsePoly1D(getFirstAndOnly(incaElem, "TimeCAPoly"), inca->timeCAPoly);
        parseDouble(getFirstAndOnly(incaElem, "R_CA_SCP"), inca->rangeCA);
        parseDouble(getFirstAndOnly(incaElem, "FreqZero"), inca->freqZero);
        parsePoly1D(getFirstAndOnly(incaElem, "DRateSFPoly"),
                    inca->dopplerRateScaleFactorPoly);

        XMLElem tmpElem = getOptional(incaElem, "DopCentroidPoly");
        if (tmpElem)
        {
            parsePoly2D(tmpElem, inca->dopplerCentroidPoly);
        }

        tmpElem = getOptional(incaElem, "DopCentroidCOA");
        if (tmpElem)
        {
            parseBooleanType(tmpElem, inca->dopplerCentroidCOA);
        }
    }
}

void ComplexXMLControl::parseFootprint(XMLElem footprint,
                                       std::string cornerName, std::vector<
                                               LatLon>& value, bool alt)
{
    std::vector<XMLElem> vertices;
    footprint->getElementsByTagName(cornerName, vertices);

    value.clear();
    value.resize(4);

    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        //check the index attr to know which corner it is
        int
                idx =
                        str::toType<int>(
                                         vertices[i]->getAttributes().getValue(
                                                                               "index").substr(
                                                                                               0,
                                                                                               1))
                                - 1;

        parseLatLon(vertices[i], value[idx]);

        if (alt)
        {
            double hae;

            parseDouble(getFirstAndOnly(vertices[i], "HAE"), hae);
            value[idx].setAlt(hae);
        }
    }
}

XMLElem ComplexXMLControl::createFootprint(std::string name,
                                           std::string cornerName,
                                           const std::vector<LatLon>& corners,
                                           bool alt, XMLElem parent)
{
    XMLElem footprint = newElement(name, parent);

    XMLElem vertex = createLatLon(cornerName, corners[0], footprint);
    setAttribute(vertex, "index", "1:FRFC");

    vertex = createLatLon(cornerName, corners[1], footprint);
    setAttribute(vertex, "index", "2:FRLC");

    vertex = createLatLon(cornerName, corners[2], footprint);
    setAttribute(vertex, "index", "3:LRLC");

    vertex = createLatLon(cornerName, corners[3], footprint);
    setAttribute(vertex, "index", "4:LRFC");

    return footprint;
}

XMLElem ComplexXMLControl::createString(std::string name, std::string uri,
                                        std::string p, XMLElem parent)
{
    XMLElem e = XMLControl::createString(name, uri, p, parent);
    if (e)
    {
        // add the class attribute
        xml::lite::AttributeNode node;
        node.setQName("class");
        node.setUri(getDefaultURI());
        node.setValue("xs:string");
        e->getAttributes().add(node);
    }
    return e;
}

XMLElem ComplexXMLControl::createInt(std::string name, std::string uri, int p,
                                     XMLElem parent)
{
    XMLElem e = XMLControl::createInt(name, uri, p, parent);
    if (e)
    {
        // add the class attribute
        xml::lite::AttributeNode node;
        node.setQName("class");
        node.setUri(getDefaultURI());
        node.setValue("xs:int");
        e->getAttributes().add(node);
    }
    return e;
}

XMLElem ComplexXMLControl::createDouble(std::string name, std::string uri,
                                        double p, XMLElem parent)
{
    XMLElem e = XMLControl::createDouble(name, uri, p, parent);
    if (e)
    {
        // add the class attribute
        xml::lite::AttributeNode node;
        node.setQName("class");
        node.setUri(getDefaultURI());
        node.setValue("xs:double");
        e->getAttributes().add(node);
    }
    return e;
}

XMLElem ComplexXMLControl::createBooleanType(std::string name, std::string uri,
                                             BooleanType p, XMLElem parent)
{
    XMLElem e = XMLControl::createBooleanType(name, uri, p, parent);
    if (e)
    {
        // add the class attribute
        xml::lite::AttributeNode node;
        node.setQName("class");
        node.setUri(getDefaultURI());
        node.setValue("xs:boolean");
        e->getAttributes().add(node);
    }
    return e;
}

XMLElem ComplexXMLControl::createDateTime(std::string name, std::string uri,
                                          std::string s, XMLElem parent)
{
    XMLElem e = XMLControl::createDateTime(name, uri, s, parent);
    if (e)
    {
        // add the class attribute
        xml::lite::AttributeNode node;
        node.setQName("class");
        node.setUri(getDefaultURI());
        node.setValue("xs:dateTime");
        e->getAttributes().add(node);
    }
    return e;
}

XMLElem ComplexXMLControl::createDateTime(std::string name, std::string uri,
                                          DateTime p, XMLElem parent)
{
    std::string s = six::toString<DateTime>(p);
    return createDateTime(name, uri, s, parent);
}

XMLElem ComplexXMLControl::createDate(std::string name, std::string uri,
                                      DateTime p, XMLElem parent)
{
    XMLElem e = XMLControl::createDate(name, uri, p, parent);
    if (e)
    {
        // add the class attribute
        xml::lite::AttributeNode node;
        node.setQName("class");
        node.setUri(getDefaultURI());
        node.setValue("xs:date");
        e->getAttributes().add(node);
    }
    return e;
}

XMLElem ComplexXMLControl::createString(std::string name, std::string p,
                                        XMLElem parent)
{
    return createString(name, getDefaultURI(), p, parent);
}

XMLElem ComplexXMLControl::createInt(std::string name, int p, XMLElem parent)
{
    return createInt(name, getDefaultURI(), p, parent);
}

XMLElem ComplexXMLControl::createDouble(std::string name, double p,
                                        XMLElem parent)
{
    return createDouble(name, getDefaultURI(), p, parent);
}

XMLElem ComplexXMLControl::createBooleanType(std::string name, BooleanType p,
                                             XMLElem parent)
{
    return createBooleanType(name, getDefaultURI(), p, parent);
}

XMLElem ComplexXMLControl::createDateTime(std::string name, std::string s,
                                          XMLElem parent)
{
    return createDateTime(name, getDefaultURI(), s, parent);
}

XMLElem ComplexXMLControl::createDateTime(std::string name, DateTime p,
                                          XMLElem parent)
{
    return createDateTime(name, getDefaultURI(), p, parent);
}

XMLElem ComplexXMLControl::createDate(std::string name, DateTime p,
                                      XMLElem parent)
{
    return createDate(name, getDefaultURI(), p, parent);
}
