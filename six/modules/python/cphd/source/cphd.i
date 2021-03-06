/*
 * =========================================================================
 * This file is part of cphd-python
 * =========================================================================
 *
 * (C) Copyright 2004 - 2020, MDA Information Systems LLC
 *
 * cphd-python is free software; you can redistribute it and/or modify
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
 */

%module(package="pysix") cphd

%feature("autodoc", "1");
%feature("flatnested");

%include "stdint.i"
%include <std_string.i>
%include <std_vector.i>

%import "sys.i"
%import "types.i"
%import "mem.i"
%import "six_sicd.i"
%import "six.i"
%import "math_poly.i"
%import "math_linear.i"
%import "scene.i"

%{
#include "import/types.h"
#include "import/cphd.h"
#include "import/six.h"
#include "import/six/sicd.h"
#include "import/sys.h"
#include <numpyutils/numpyutils.h>

using six::Vector3;
%}

%ignore cphd::CPHDXMLControl::toXML(const Metadata& metadata);
%ignore cphd::CPHDXMLControl::toXML(
    const Metadata& metadata,
    const std::vector<std::string>& schemaPaths);

%ignore cphd::CPHDXMLControl::fromXML(const xml::lite::Document* doc);
%ignore cphd::CPHDXMLControl::fromXML(
    const xml::lite::Document* doc,
    const std::vector<std::string>& schemaPaths);

%ignore cphd::CPHDXMLControl::fromXML(const std::string& xmlString);
%ignore cphd::CPHDXMLControl::fromXML(
    const std::string& xmlString,
    const std::vector<std::string>& schemaPaths);

// Nested class renames
%rename(CphdAntenna) cphd::Antenna;
%rename(DataChannel) cphd::Data::Channel;
%rename(DataSupportArray) cphd::Data::SupportArray;
%rename(ChannelTxRcv) cphd::ChannelParameter::TxRcv;
%rename(ChannelAntenna) cphd::ChannelParameter::Antenna;
%rename(TOALFMEclipse) cphd::TOAExtended::LFMEclipse;
%rename(ErrorMonostatic) cphd::ErrorParameters::Monostatic;
%rename(ErrorMonostaticRadarSensor) cphd::ErrorParameters::Monostatic::RadarSensor;
%rename(ErrorBistatic) cphd::ErrorParameters::Bistatic;
%rename(ErrorBistaticRadarSensor) cphd::ErrorParameters::Bistatic::RadarSensor;

%include "cphd/Types.h"
%include "cphd/Enums.h"
%include "cphd/MetadataBase.h"
%include "cphd/FileHeader.h"
%include "cphd/Global.h"
%include "cphd/SceneCoordinates.h"
%include "cphd/ReferenceGeometry.h"
%include "cphd/SupportArray.h"
%include "cphd/ErrorParameters.h"
%include "cphd/ProductInfo.h"
%include "cphd/SupportBlock.h"
%include "cphd/Channel.h"
%include "cphd/Data.h"
%include "cphd/Dwell.h"
%include "cphd/TxRcv.h"
%include "cphd/PVP.h"
%include "cphd/Antenna.h"
%include "cphd/Metadata.h"
%include "cphd/PVPBlock.h"
%include "cphd/CPHDXMLControl.h"
%include "cphd/Wideband.h"
%include "cphd/CPHDReader.h"
%include "cphd/CPHDWriter.h"

%ignore CPHDWriter::writeCPHDData;
%ignore CPHDWriter::close;
%ignore CPHDWriter::writeMetadata;
%ignore CPHDWriter::write;

%extend cphd::FileHeader
{
    std::string __str__()
    {
        std::ostringstream out;
        out << *$self;
        return out.str();
    }
}

%extend cphd::Metadata
{
    std::string __str__()
    {
        std::ostringstream out;
        out << *$self;
        return out.str();
    }
}

%extend cphd::PVPBlock
{
    std::string __str__()
    {
        std::ostringstream out;
        out << *$self;
        return out.str();
    }
}

%extend cphd::PVPBlock
{
    void getPVPdata(size_t channel, size_t data)
    {
        void* buffer = reinterpret_cast<void*>(data);
        $self->getPVPdata(channel, buffer);
    }
}

%include "std_map.i"
%template(MapStringAPVPType) std::map<std::string, cphd::APVPType>;

%extend cphd::PVPBlock
{
// Types from Table 10-2 of the CPHD spec
%template(setF4AddedPVP) setAddedPVP<float>;
%template(setF8AddedPVP) setAddedPVP<double>;
%template(setU1AddedPVP) setAddedPVP<sys::Uint8_T>;
%template(setU2AddedPVP) setAddedPVP<sys::Uint16_T>;
%template(setU4AddedPVP) setAddedPVP<sys::Uint32_T>;
%template(setU8AddedPVP) setAddedPVP<sys::Uint64_T>;
%template(setI1AddedPVP) setAddedPVP<sys::Int8_T>;
%template(setI2AddedPVP) setAddedPVP<sys::Int16_T>;
%template(setI4AddedPVP) setAddedPVP<sys::Int32_T>;
%template(setI8AddedPVP) setAddedPVP<sys::Int64_T>;
%template(setCI2AddedPVP) setAddedPVP<std::complex<sys::Int8_T> >;
%template(setCI4AddedPVP) setAddedPVP<std::complex<sys::Int16_T> >;
%template(setCI8AddedPVP) setAddedPVP<std::complex<sys::Int32_T> >;
%template(setCI16AddedPVP) setAddedPVP<std::complex<sys::Int64_T> >;
%template(setCF8AddedPVP) setAddedPVP<std::complex<float> >;
%template(setCF16AddedPVP) setAddedPVP<std::complex<double> >;
%template(setStringAddedPVP) setAddedPVP<std::string>;
}

%extend cphd::PVPBlock
{
%pythoncode
%{

    import numpy  # 'as np' doesn't work unless the import is in each method

    @staticmethod
    def _validate_pvp_format(format_str):
        if format_str.startswith('CI'):
            # NumPy has no complex int type, need to use a structured array
            # TODO: Support complex int PVP types
            raise NotImplementedError('Complex int PVPs are not supported')

        if '=' in format_str and ';' in format_str:
            # Multiple values in parameter
            param_types = [param[param.index('=') + 1:]
                           for param in format_str.split(';') if param]
            if not len(set(param_types)) == 1:
                # TODO: support multiple different parameter types e.g.
                #       ('A=U2;B=I2;') using NumPy structured arrays
                raise ValueError('PVPs with different types are not supported')
            return param_types[0]
        return format_str

    @staticmethod
    def _format_to_dtype(format_str):
        format_str = PVPBlock._validate_pvp_format(format_str)

        first = format_str[0]
        if first in ['U', 'I', 'F']:  # Unsigned int, signed int, float
            return numpy.dtype(format_str.lower())
        elif first == 'C':  # Complex float ('CF') or complex int ('CI')
            if format_str[1] == 'I':  # Complex int
                raise NotImplementedError('Complex int PVPs are not supported')
            return numpy.dtype('c' + format_str[2:])
        elif first == 'S':  # String
            return numpy.dtype('U' + format_str[1:])

        raise ValueError('Unknown or unsupported format string: {0}'
                         .format(format_str))

    @staticmethod
    def _common_format(formats):
        # Split parameters into a format dict with keys equal to
        # param suffixes. Single parameters have entry suffix
        # Example: 'F8' --> {'': 'F8'}
        # Example: 'A=F8;B=I4' --> {'A': 'F8', 'B': 'I4'}
        return len(set(formats)) == 1

    @staticmethod
    def _split_multi_format(fmt):
        if ';' in fmt:
            result = {}
            for _fmt in fmt.split(';')[:fmt.count(';')]:
                assert '=' in _fmt
                name, __fmt = _fmt.split('=')
                result[name] = __fmt
            return result
        return {'': fmt}

    def to_list_of_dicts(self, metadata):
        """Turn this PVPBlock into a list of Python dicts of NumPy arrays.

        Parameters
        ----------
        metadata: cphd.Metadata
            The metadata used to create this PVPBlock

        Returns
        -------
        list_of_dicts: list of dicts of NumPy arrays
            Each dictionary in the list corresponds to a CPHD data channel.
            The dictionary keys are string names of PVP parameters
            The dictionary values are NumPy arrays of shape
              (metadata.getNumVectors(channel),metadata.getNumSamples(channel))
            Any added PVP parameters should also have been added to
              metadata.pvp.addedPVP
        """
        pvp_info = {}

        # Methods to check if an optional PVP param is set
        # Using .lower() since name capitalization is not 100% consistent
        has_optional_param_methods = {name.lower(): getattr(self, name)
                                      for name in dir(self)
                                      if name.startswith('has')}

        # Set required and optional PVP parameters
        for name in dir(metadata.pvp):
            try:
                attr = getattr(metadata.pvp, name)
            except AttributeError:
                continue
            if not isinstance(attr, pysix.cphd.PVPType):
                continue
            if attr.getSize() == 0:
                continue
            # Is this an optional parameter that is not set?
            if ('has' + name.lower()) in has_optional_param_methods \
                    and not has_optional_param_methods['has' + name.lower()]():
                continue
            pvp_info[name] = (
                attr.getSize(), attr.getOffset(), attr.getFormat()
            )
        # Set any added PVP parameters
        for name, added_pvp in metadata.pvp.addedPVP.items():
            if added_pvp.getSize() == 0:
                continue
            PVPBlock._validate_pvp_format(added_pvp.getFormat())
            pvp_info[name] = (added_pvp.getSize(),
                              added_pvp.getOffset(),
                              added_pvp.getFormat())

        # Sort pvp_info by offset
        from collections import OrderedDict  # SWIG won't import this above
        sorted_pvp_info = OrderedDict(
            sorted(pvp_info.items(), key=lambda item: item[1][1])
        )

        pvp_data = []
        # Read data from each channel of this PVPBlock into list-of-dicts
        for channel in range(metadata.getNumChannels()):
            channel_pvp = {}

            num_vectors = metadata.getNumVectors(channel)
            pvp_size = metadata.pvp.sizeInBytes()

            channel_data = np.empty(
                shape=num_vectors * pvp_size, dtype=np.bytes_
            )
            self.getPVPdata(
                channel,channel_data.__array_interface__['data'][0]
            )
            channel_data = channel_data.reshape(num_vectors, pvp_size)

            pvp_offset = 0
            for name, (size, offset, fmt) in sorted_pvp_info.items():
                suffix_to_format = PVPBlock._split_multi_format(fmt)

                # Number of values in this parameter, not necessarily equal to
                # _size (e.g. 'CF16' uses 2 8-byte words for 1 value)
                # Store this in case suffix_to_format changes below
                num_values = len(suffix_to_format)

                # If a PVP has multiple parameters but all have the same
                # format, collapse them into a single PVP. This is intended
                # to simplify things like 'txPos', where it's more convenient
                # to have an (N,3)-shape array than three length-N arrays
                # for 'txPosX', 'txPosY', and 'txPosZ' separately.
                _size = 1
                formats = list(suffix_to_format.values())
                if PVPBlock._common_format(formats):
                    suffix_to_format = {'': formats[0]}
                    _size = size

                for suffix, _fmt in suffix_to_format.items():
                    _name = name + suffix
                    dtype = PVPBlock._format_to_dtype(_fmt)
                    length = dtype.itemsize * num_values

                    # For every vector (first index of channel_data), slice
                    # the range with values for this parameter (second index)
                    vals = channel_data[
                        :, pvp_offset:(pvp_offset + length)
                    ].copy().view(dtype)
                    channel_pvp[_name] = vals.reshape(
                        num_vectors, num_values
                    ).squeeze()
                    pvp_offset += 8 * _size  # Move to next 8-byte word

            pvp_data.append(channel_pvp)

        return pvp_data

    @staticmethod
    def from_list_of_dicts(pvp_data, metadata):
        """Initialize and populate a PVPBlock from a list of Python dicts.

        Parameters
        ----------
        pvp_data: list of Python dicts
            List of Python dicts (one for each channel) mapping parameter names
            to NumPy arrays of data. See PVPBlock.toListOfDicts() for more info
            on the structure expected here
        metadata: cphd.Metadata
            Metadata used to create this PVPBlock
        """
        pvp_block = PVPBlock(metadata.pvp, metadata.data)

        # Methods to check if an optional PVP param is set
        # Using .lower() since name capitalization is not 100% consistent
        has_optional_param_methods = {name.lower(): getattr(pvp_block, name)
                                      for name in dir(pvp_block)
                                      if name.startswith('has')}

        pvp_info = {}

        # Set required and optional PVP parameters
        for name in dir(metadata.pvp):
            try:
                attr = getattr(metadata.pvp, name)
            except AttributeError:
                continue
            if not isinstance(attr, pysix.cphd.PVPType):
                continue
            if attr.getSize() == 0:
                continue
            # Is this an optional parameter that is not set?
            if ('has' + name.lower()) in has_optional_param_methods \
                    and not has_optional_param_methods['has' + name.lower()]():
                continue
            pvp_info[name] = (
                attr.getSize(), attr.getOffset(), attr.getFormat()
            )

        # Using .lower() since name capitalization is not 100% consistent
        default_params_in_use = [name.lower() for name in pvp_info]
        default_param_setters = {
            name[3:].lower(): getattr(pvp_block, name)
            for name in dir(pvp_block)
            if name.startswith('set')
            and name[3:].lower() in default_params_in_use
        }

        # Populate PVPBlock object from pvp_data
        for channel_index, channel_data in enumerate(pvp_data):
            for vector_index in range(metadata.getNumVectors(channel_index)):
                for name, data in channel_data.items():
                    vector_data = data[vector_index]
                    if isinstance(vector_data, numpy.ndarray):
                        if len(vector_data) == 3:
                            vector_data = coda.math_linear.Vector3(vector_data)
                        else:
                            raise ValueError(
                                ('Only PVP parameters of size 1 or 3 are '
                                 'supported, {0} has size {1}')
                                .format(name, len(vector_data))
                            )
                    if 'numpy' in type(vector_data).__module__:
                        # Change 1D arrays to scalars AND convert NumPy types
                        # (e.g. np.int64) to Python types that SWIG understands
                        vector_data = vector_data.item()
                    if name not in metadata.pvp.addedPVP:
                        # Get and call the setter method for this parameter
                        # Use .lower() since capitalization isn't 100% the same
                        default_param_setters[name.lower()](
                            vector_data, channel_index, vector_index)
                    else:
                        param_format = metadata.pvp.addedPVP[name].getFormat()
                        PVPBlock._validate_pvp_format(param_format)

                        # Get and call setter method for the parameter type
                        getattr(pvp_block, 'set' + param_format + 'AddedPVP')(
                            vector_data, channel_index, vector_index, name)
        return pvp_block
%}
}

%extend cphd::Wideband
{
    // We need to expose a way to read into a raw buffer
    void readImpl(size_t channel,
                  size_t firstVector,
                  size_t lastVector,
                  size_t firstSample,
                  size_t lastSample,
                  size_t numThreads,
                  const types::RowCol<size_t>& dims,
                  long long data)
    {
        $self->read(channel,
                    firstVector,
                    lastVector,
                    firstSample,
                    lastSample,
                    numThreads,
                    dims,
                    reinterpret_cast<void*>(data));
    }
}

%extend cphd::CPHDReader
{
    PyObject* getPHD(size_t channel)
    {
        const auto& wb = self->getWideband();
        const types::RowCol<size_t> dims =
            wb.getBufferDims(channel,
                             0,
                             cphd::Wideband::ALL,
                             0,
                             cphd::Wideband::ALL);
        std::vector<std::complex<float>> buffer(dims.area());
        wb.read(channel,
                0,
                cphd::Wideband::ALL,
                0,
                cphd::Wideband::ALL,
                1,
                dims,
                buffer.data());
        return numpyutils::toNumpyArray(
            dims.row, dims.col, NPY_COMPLEX64, buffer.data());
    }
}

%extend cphd::CPHDWriter
{

%pythoncode
%{
    def __del__(self):
        self.close()
%}

/*! Write a sequence of NumPy arrays of wideband data to a CPHD file
 * \param metadata (cphd::Metadata)
 * \param pvpBlock (cphd::PVPBlock)
 * \param widebandArray
 *        A sequence of NumPy arrays of wideband data, one array per channel
 * \param supportData (sys::ubyte*)
          A pointer to a contiguous block of support array data.
          Defaults to nullptr
 */
void writeWideband(const Metadata& metadata,
                   const PVPBlock& pvpBlock,
                   PyObject* widebandArrays,
                   PyObject* supportArrays=nullptr)
{
    const size_t numChannels = static_cast<size_t>(
        PySequence_Length(widebandArrays)
    );
    if (numChannels != metadata.data.getNumChannels())
    {
        std::ostringstream oss;
        oss << "Number of channels in metadata ("
            << metadata.data.getNumChannels() << ") "
            << "does not match number of channels received ("
            << numChannels << ")";
        throw except::Exception(Ctxt(oss.str()));
    }

    $self->writeMetadata(pvpBlock);

    if (metadata.data.getNumSupportArrays() != 0)
    {
        if (supportArrays == nullptr)
        {
            throw except::Exception(Ctxt("Support data is not provided"));
        }
        // TODO: This has not been tested by reading support arrays back in
        $self->writeSupportData(supportArrays);
    }

    $self->writePVPData(pvpBlock);

    for (size_t ii = 0; ii < numChannels; ++ii)
    {
        PyObject* channelWideband = PySequence_GetItem(widebandArrays, ii);

        // Note that different channels could have different dimensions
        const types::RowCol<size_t> metadataDims(
            metadata.data.getNumVectors(ii),
            metadata.data.getNumSamples(ii)
        );
        const types::RowCol<size_t> arrayDims = \
            numpyutils::getDimensionsRC(channelWideband);
        if (metadataDims != arrayDims)
        {
            std::ostringstream oss;
            oss << "For channel " << ii << ", wideband dimensions ("
                << arrayDims.row << ", " << arrayDims.col << ") "
                << "do not match dimensions in metadata: ("
                << metadataDims.row << ", " << metadataDims.col << ")";
            throw except::Exception(Ctxt(oss.str()));
        }

        const size_t numElements = arrayDims.row * arrayDims.col;
        std::complex<float>* widebandPtr = \
            numpyutils::getBuffer<std::complex<float>>(channelWideband);
        $self->writeCPHDData<std::complex<float>>(
            widebandPtr, numElements, ii
        );
    }
}

}

%pythoncode
%{
    import os
    import numpy as np

    def write_cphd(
            pathname,
            metadata,
            pvp_block,
            wideband_arrays,
            support_arrays=None,
            schema_paths=None,
            num_threads=None,
            scratch_space=None):
        """Write CPHD data to a file.

        Parameters
        ----------
        pathname: str
        metadata: cphd.Metadata
        pvp_block: cphd.PVPBlock
        wideband_arrays: sequence of np.arrays
            Arrays of wideband data, one per channel
        support_arrays (sequence of np.arrays)
            Arrays of support array data
        schema_paths: list of strs
        num_threads: int
        scratch_space: int
        """
        if schema_paths is None:
            if 'SIX_SCHEMA_PATH' in os.environ:
                schema_paths = [os.environ['SIX_SCHEMA_PATH']]
            else:
                schema_paths = []
        if num_threads is None:
            num_threads = 0
        if scratch_space is None:
            scratch_space = 4 * 1024 * 1024


        # Support writing a single channel with a single ndarray
        if isinstance(wideband_arrays, np.ndarray):
            wideband_arrays = [wideband_arrays]

        if support_arrays:
            # Support writing a single support array with a single ndarray
            if isinstance(support_arrays, np.ndarray):
                support_arrays = [support_arrays]

            # Flatten (possibly differently-shaped) support arrays into 1D,
            # stack them, then assert that the data is contiguous in memory
            support_arrays = [np.ndarray.flatten(support_array)
                              for support_array in support_arrays]
            support_arrays = np.stack(support_arrays)
            if not support_arrays.flags['C_CONTIGUOUS']:
                support_arrays = np.ascontiguousarray(support_arrays)
            assert support_arrays.flags['C_CONTIGUOUS'], \
                   'Could not make support array data contiguous'

        writer = CPHDWriter(
            metadata, pathname, schema_paths, num_threads, scratch_space
        )
        writer.writeWideband(
            metadata, pvp_block, wideband_arrays, support_arrays
        )
%}

%pythoncode
%{
import numpy
import multiprocessing
from coda.coda_types import RowColSizeT

def read(self,
         channel=0,
         first_vector=0,
         last_vector=Wideband.ALL,
         first_sample=0,
         last_sample=Wideband.ALL,
         num_threads=multiprocessing.cpu_count()):
    """Write Wideband data to a NumPy array.

    Parameters
    ----------
    channel: int
    first_vector: int
    last_vector: int
    first_sample: int
    last_sample: int
    num_threads: int

    Returns
    -------
    wideband: np.array
    """
    dims = self.getBufferDims(
        channel, first_vector, last_vector, first_sample, last_sample
    )
    sample_type_size = self.getElementSize()

    # RF32F_IM32F
    if sample_type_size == 8:
        dtype = 'complex64'
    else:
        raise Exception('Unknown element type')

    wideband = numpy.empty(shape=(dims.row, dims.col), dtype=dtype)
    pointer, ro = wideband.__array_interface__['data']
    self.readImpl(channel,
                  first_vector,
                  last_vector,
                  first_sample,
                  last_sample,
                  num_threads,
                  dims,
                  pointer)
    return wideband

Wideband.read = read
%}

%extend cphd::CPHDXMLControl {
  cphd::Metadata fromXMLString(const std::string& xmlString)
  {
    cphd::Metadata retv;
    std::unique_ptr<cphd::Metadata> apMetadata = $self->fromXML(xmlString);
    retv = *apMetadata;
    return retv;
  }
}
%template(VectorVector2) std::vector<math::linear::VectorN<2,double> >;
%template(VectorVector3) std::vector<math::linear::VectorN<3,double> >;
%template(VectorString) std::vector<std::string>;
%template(VectorLineSample) std::vector<cphd::LineSample>;
%template(VectorSegment) std::vector<cphd::Segment>;
%template(VectorChannelParameters) std::vector<cphd::ChannelParameter>;
%template(VectorPoint) std::vector<cphd::Point>;
%template(VectorDataChannel) std::vector<cphd::Data::Channel>;
%template(VectorAntennaParameters) std::vector<six::sicd::AntennaParameters>;
%template(VectorSupportArrayParameter) std::vector<cphd::SupportArrayParameter>;
%template(VectorCOD) std::vector<cphd::COD>;
%template(VectorDwellTime) std::vector<cphd::DwellTime>;
%template(VectorAntCoordFrame) std::vector<cphd::AntCoordFrame>;
%template(VectorAntPhaseCenter) std::vector<cphd::AntPhaseCenter>;
%template(VectorAntPattern) std::vector<cphd::AntPattern>;
%template(VectorGainPhaseArray) std::vector<cphd::AntPattern::GainPhaseArray>;
%template(VectorTxParams) std::vector<cphd::TxWFParameters>;
%template(VectorRcvParams) std::vector<cphd::RcvParameters>;
%template(VectorCreationInfo) std::vector<cphd::ProductInfo::CreationInfo>;
%template(VectorGeoInfo) std::vector<six::GeoInfo>;

SCOPED_COPYABLE(cphd,SupportArray);
SCOPED_COPYABLE(cphd,TropoParameters);
SCOPED_COPYABLE(cphd,IonoParameters);
SCOPED_COPYABLE(cphd,Planar);
SCOPED_COPYABLE(cphd,HAE);
SCOPED_COPYABLE(cphd,AreaType);
SCOPED_COPYABLE(cphd,ImageGrid);
SCOPED_COPYABLE(cphd,TxRcv);
SCOPED_COPYABLE(cphd,TOAExtended);
SCOPED_COPYABLE_RENAME(cphd,TOAExtended::LFMEclipse,TOALFMEclipse);
SCOPED_COPYABLE_RENAME(cphd,ChannelParameter::Antenna,ChannelAntenna);
SCOPED_COPYABLE_RENAME(cphd,ChannelParameter::TxRcv,ChannelTxRcv);
SCOPED_COPYABLE(cphd,TgtRefLevel);
SCOPED_COPYABLE(cphd,NoiseLevel);
SCOPED_COPYABLE(cphd,FxNoiseProfile);
SCOPED_COPYABLE(cphd,Monostatic);
SCOPED_COPYABLE(cphd,Bistatic);
SCOPED_COPYABLE(cphd,ErrorParameters);
SCOPED_COPYABLE_RENAME(cphd,ErrorParameters::Monostatic,ErrorMonostatic);
SCOPED_COPYABLE(six,DecorrType);
SCOPED_COPYABLE(cphd,ProductInfo);
SCOPED_COPYABLE(cphd,MatchInformation);
SCOPED_COPYABLE_RENAME(cphd,Antenna,CphdAntenna);
