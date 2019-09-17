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

#include <cphd/Channel.h>
#include <six/Init.h>

namespace cphd
{

TOAExtended::TOAExtended() :
    toaExtSaved(six::Init::undefined<double>())
{
}

TOAExtended::LFMEclipse::LFMEclipse() :
    fxEarlyLow(six::Init::undefined<double>()),
    fxEarlyHigh(six::Init::undefined<double>()),
    fxLateLow(six::Init::undefined<double>()),
    fxLateHigh(six::Init::undefined<double>())
{
}

DwellTimes::DwellTimes() :
    codId(six::Init::undefined<std::string>()),
    dwellId(six::Init::undefined<std::string>())
{
}

TgtRefLevel::TgtRefLevel() :
    ptRef(six::Init::undefined<double>())
{
}

Point::Point() :
    fx(six::Init::undefined<double>()),
    pn(six::Init::undefined<double>())
{
}

NoiseLevel::NoiseLevel() :
    pnRef(six::Init::undefined<double>()),
    bnRef(six::Init::undefined<double>())
{
}

ChannelParameter::ChannelParameter() :
    identifier(six::Init::undefined<std::string>()),
    refVectorIndex(six::Init::undefined<size_t>()),
    fxFixed(six::Init::undefined<six::BooleanType>()),
    toaFixed(six::Init::undefined<six::BooleanType>()),
    srpFixed(six::Init::undefined<six::BooleanType>()),
    signalNormal(six::Init::undefined<six::BooleanType>()),
    fxC(six::Init::undefined<double>()),
    fxBW(six::Init::undefined<double>()),
    fxBWNoise(six::Init::undefined<double>()),
    toaSaved(six::Init::undefined<double>())
{
}

ChannelParameter::Antenna::Antenna() :
    txAPCId(six::Init::undefined<std::string>()),
    txAPATId(six::Init::undefined<std::string>()),
    rcvAPCId(six::Init::undefined<std::string>()),
    rcvAPATId(six::Init::undefined<std::string>())
{
}

Channel::Channel() :
    refChId(six::Init::undefined<std::string>()),
    fxFixedCphd(six::Init::undefined<six::BooleanType>()),
    toaFixedCphd(six::Init::undefined<six::BooleanType>()),
    srpFixedCphd(six::Init::undefined<six::BooleanType>())
{
}


std::ostream& operator<< (std::ostream& os, const Polarization& p)
{
    os << "      TxPol        : " << p.txPol << "\n"
        << "      RcvPol       : " << p.rcvPol << "\n";
    return os;
}

std::ostream& operator<< (std::ostream& os, const TOAExtended& t)
{
    os << "      TOAExtended:: \n"
        << "      TOAExtSaved  : " << t.toaExtSaved << "\n"
        << "      LFMEclipse:: \n"
        << "        FxEarlyLow : " << t.lfmEclipse->fxEarlyLow << "\n"
        << "        FxEarlyHigh : " << t.lfmEclipse->fxEarlyHigh << "\n"
        << "        FxLateLow : " << t.lfmEclipse->fxLateLow << "\n"
        << "        FxLateHigh : " << t.lfmEclipse->fxLateHigh << "\n";
    return os;
}

std::ostream& operator<< (std::ostream& os, const DwellTimes& d)
{
    os << "      DwellTimes:: \n"
        << "      CODId        : " << d.codId << "\n"
        << "      DwellId        : " << d.dwellId << "\n";
        return os;
}

std::ostream& operator<< (std::ostream& os, const TgtRefLevel& t)
{
    os << "      TgtRefLevel:: \n"
        << "      PtRef        : " << t.ptRef << "\n";
        return os;
}

std::ostream& operator<< (std::ostream& os, const Point& p)
{
    os << "      Point:: \n"
        << "      Fx        : " << p.fx << "\n"
        << "      Pn        : " << p.pn << "\n";
        return os;
}

std::ostream& operator<< (std::ostream& os, const FxNoiseProfile& f)
{
    os << "        FxNoiseProfile:: \n";
    for (size_t i = 0; i < f.point.size(); ++i)
    {
        os << "        Point        : " << f.point[i] << "\n";
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const NoiseLevel& n)
{
    os << "      NoiseLevel:: \n"
        << "      PnRef        : " << n.pnRef << "\n"
        << "      BnRef        : " << n.bnRef << "\n";
    if(n.fxNoiseProfile.get())
    {
        os << *(n.fxNoiseProfile) << "\n";
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const ChannelParameter& c)
{
    os << "    ChannelParameter:: \n"
        << "      Identifier   : " << c.identifier << "\n"
        << "      RefVectorIndex : " << c.refVectorIndex << "\n"
        << "      FxFixed      : " << c.fxFixed << "\n"
        << "      TOAFixed     : " << c.toaFixed << "\n"
        << "      SRPFixed     : " << c.srpFixed << "\n"
        << "      SginalNormal : " << c.signalNormal << "\n"
        << "      Polarization:: \n"
        << c.polarization << "\n"
        << "      FxC          : " << c.fxC << "\n"
        << "      FxBW         : " << c.fxBW << "\n"
        << "      FxBWNoise    : " << c.fxBWNoise << "\n"
        << "      TOASaved     : " << c.toaSaved << "\n";
    if (c.toaExtended.get())
    {
        os << *(c.toaExtended) << "\n";
    }
    os << c.dwellTimes << "\n"
        << c.imageArea << "\n"
        << "      Antenna:: \n"
        << "      TxAPCId      : " << c.antenna->txAPCId << "\n"
        << "      TxAPATId     : " << c.antenna->txAPATId << "\n"
        << "      RcvAPCId     : " << c.antenna->rcvAPCId << "\n"
        << "      RcvAPATId    : " << c.antenna->rcvAPATId << "\n"
        << "      TxRcv:: \n";
    for (size_t i = 0; i < c.txRcv->txWFId.size(); ++i)
    {
        os << "      TxWFId       : " << c.txRcv->txWFId[i] << "\n";
    }
    for (size_t i = 0; i < c.txRcv->rcvId.size(); ++i)
    {
        os << "      RcvId       : " << c.txRcv->rcvId[i] << "\n";
    }
    if (c.tgtRefLevel.get())
    {
        os << *(c.tgtRefLevel) << "\n";
    }
    if (c.noiseLevel.get())
    {
        os << *(c.noiseLevel) << "\n";
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const Channel& c)
{
    os << "Channel:: \n"
        << "  RefChId          : " << c.refChId << "\n"
        << "  FxFixedCphd      : " << c.fxFixedCphd << "\n"
        << "  TOAFixedCphd     : " << c.toaFixedCphd << "\n"
        << "  SRPFixedCphd     : " << c.srpFixedCphd << "\n"
        << "  Parameters:: \n";
    for (size_t i = 0; i < c.parameters.size(); ++i)
    {
        os << c.parameters[i] << "\n";
    }
    for (size_t i = 0; i < c.addedParameters.size(); ++i)
    {
        os << "  Parameter name   : " << c.addedParameters[i].getName() << "\n";
        os << "  Parameter value   : " << c.addedParameters[i].str() << "\n";
    }
    return os;
}
}