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
#ifndef __SIX_XML_CONTROL_H__
#define __SIX_XML_CONTROL_H__

#include "six/Data.h"
#include "six/Parameter.h"
#include "six/ErrorStatistics.h"
#include "six/Radiometric.h"
#include <import/xml/lite.h>

namespace six
{

/*!
 *  \class XMLControl
 *  \brief Base class for reading and writing a Data object
 *
 *  This interfaces' implementors are not typically called directly.
 *  Instead, they are used to convert back and forth from XML during
 *  file format reads and writes.
 *
 *  The XMLControl is derived for each Data type that is supported
 *  in this library.  This class does not actually convert data into
 *  a bytes, although the helper functions in the XMLControlFactory do.
 *
 *  Instead, this class defines the interface for going between a data
 *  model, represented by the Data object, to an XML Document Object Model
 *  (DOM), and back.
 *
 *  This interface's implementors are used by the ReadControl and the
 *  WriteControl to bundle the XML content into a file container.
 *
 *  They can also be used to interact with an XML model or a stub XML
 *  file as well.
 */
class XMLControl
{

public:
    //!  Constructor
    XMLControl()
    {
    }
    //!  Destructor
    virtual ~XMLControl()
    {
    }

    /*!
     *  Convert the Data model into an XML DOM.
     *  \param data the Data model
     *  \return An XML DOM
     */
    virtual xml::lite::Document* toXML(Data* data) = 0;
    
    /*!
     *  Convert a document from a DOM into a Data model
     *  \param doc
     *  \return a Data model
     */
    virtual Data* fromXML(xml::lite::Document* doc) = 0;

protected:
    std::string mURI;

    xml::lite::Element* createString(xml::lite::Document* doc,
            std::string name, std::string p = "");
    xml::lite::Element* createInt(xml::lite::Document* doc, std::string name,
            int p = 0);
    xml::lite::Element* createDouble(xml::lite::Document* doc,
            std::string name, double p = 0);
    xml::lite::Element* createComplex(xml::lite::Document* doc,
            std::string name, std::complex<double> c);
    xml::lite::Element* createBoolean(xml::lite::Document* doc,
            std::string name, bool b = false);
    xml::lite::Element* createDateTime(xml::lite::Document* doc,
            std::string name, DateTime p);
    xml::lite::Element* createDateTime(xml::lite::Document* doc,
            std::string name, std::string s);
    xml::lite::Element* createDate(xml::lite::Document* doc, std::string name,
            DateTime p);

    xml::lite::Element* createVector3D(xml::lite::Document* doc,
            std::string name, Vector3 p = 0);
    xml::lite::Element* createRowCol(xml::lite::Document* doc,
            std::string name, int r = 0, int c = 0);
    xml::lite::Element* createRowCol(xml::lite::Document* doc,
            std::string name, double r = 0, double c = 0);
    xml::lite::Element* createRangeAzimuth(xml::lite::Document* doc,
            std::string name, double rg = 0, double az = 0);

    virtual xml::lite::Element* createFootprint(xml::lite::Document* doc,
            std::string name, std::string cornerName, Corners c, bool alt =
                    false);

    xml::lite::Element
    * createPoly1D(xml::lite::Document* doc, std::string name, Poly1D& poly1D);

    xml::lite::Element
    * createPolyXYZ(xml::lite::Document* doc, std::string name,
            PolyXYZ& polyXYZ);

    void parsePolyXYZ(xml::lite::Element* polyXML, PolyXYZ& polyXYZ);

    xml::lite::Element
    * createPoly2D(xml::lite::Document* doc, std::string name, Poly2D& poly2D);

    void parsePoly1D(xml::lite::Element* polyXML, Poly1D& poly1D);

    void parsePoly2D(xml::lite::Element* polyXML, Poly2D& poly2D);

    xml::lite::Element* newElement(xml::lite::Document* doc, std::string name)
    {
        return doc->createElement(name, mURI);
    }

    void parseVector3D(xml::lite::Element* vecXML, Vector3& vec);
    void parseLatLonAlt(xml::lite::Element* llaXML, LatLonAlt& lla);
    void parseLatLons(xml::lite::Element* pointsXML, std::string pointName,
            std::vector<LatLon>& llVec);
    virtual void parseFootprint(xml::lite::Element* footprint,
            std::string cornerName, Corners* c, bool alt = false);
    void parseParameters(xml::lite::Element* paramXML, std::string paramName,
            std::vector<Parameter>& props);

    void setAttribute(xml::lite::Element* e, std::string name, std::string v);

    static xml::lite::Element* getOptional(xml::lite::Element* parent,
            std::string tag);
    static xml::lite::Element* getFirstAndOnly(xml::lite::Element* parent,
            std::string tag);

    void addParameters(xml::lite::Document* doc, xml::lite::Element* parent,
            std::string name, std::vector<Parameter>& props);

    void addDecorrType(xml::lite::Document* doc, xml::lite::Element* parent,
            std::string name, DecorrType& decorrType);
    void parseDecorrType(xml::lite::Element* decorrXML, DecorrType& decorrType);

    xml::lite::Element* errorStatisticsToXML(xml::lite::Document* doc,
            ErrorStatistics* errorStatistics);

    void xmlToErrorStatistics(xml::lite::Element* errorStatsXML,
            ErrorStatistics* errorStatistics);

    xml::lite::Element* radiometricToXML(xml::lite::Document* doc,
            Radiometric *obj);
    void xmlToRadiometric(xml::lite::Element* radiometricXML, Radiometric *obj);

};

}

#endif
