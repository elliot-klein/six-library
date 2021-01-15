#!/usr/bin/env python

#
# =========================================================================
# This file is part of cphd-python
# =========================================================================
#
# (C) Copyright 2004 - 2020, MDA Information Systems LLC
#
# cphd-python is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; If not,
# see <http://www.gnu.org/licenses/>.
#

# In general, if functionality in CPHD is borrowed from six.sicd,
# refer to six.sicd's test script for more verification

import multiprocessing
import os
import sys
import tempfile

import numpy as np

from pysix.cphd import PVPBlock, CPHDWriter, CPHDReader
from coda.coda_types import VectorString

from util import get_test_metadata, get_test_pvp_data, get_test_widebands

NUM_THREADS = multiprocessing.cpu_count() // 2
SCRATCH_SPACE = 4 * 1024 * 1024


def main():
    metadata, support_arrays = get_test_metadata(has_support_array=True, is_compressed=False)
    widebands = get_test_widebands(metadata)
    pvp_block = PVPBlock.from_list_of_dicts(get_test_pvp_data(metadata), metadata)

    schema_paths = VectorString()
    schema_paths.push_back(os.environ['SIX_SCHEMA_PATH'])

    with tempfile.NamedTemporaryFile() as temp_file:
        cphd_filepath = temp_file.name

        cphd_writer = CPHDWriter(metadata, cphd_filepath, schema_paths, NUM_THREADS)

        # writeWideband() writes complete CPHD: XML metadata, PVP data, and wideband data
        cphd_writer.writeWideband(metadata, pvp_block, widebands, support_arrays)

        # Check that we correctly wrote the wideband data
        reader = CPHDReader(cphd_filepath, SCRATCH_SPACE)
        reader_support_arrays = reader.get_support_arrays(NUM_THREADS)  # support arrays are in np.bytes_

        if reader.getMetadata() != metadata:
            print('Test failed, original metadata and metadata from file differ')
            sys.exit(1)
        if reader.getPVPBlock() != pvp_block:
            print('Test failed, original PVP block and PVP block from file differ')
            sys.exit(1)

        for channel in range(metadata.getNumChannels()):
            if not np.array_equal(reader.getWideband().read(channel=channel), widebands[channel]):
                print('Test failed, original wideband and wideband from file differ in channel {0}'
                      .format(channel))
                sys.exit(1)
            if not np.array_equal(reader.getPHD(channel), widebands[channel]):
                print('Test failed, original wideband and PHD from file differ in channel {0}'
                      .format(channel))
                sys.exit(1)
            if not np.array_equal(reader_support_arrays[channel].view(support_arrays[channel].dtype),
                                  support_arrays[channel]):
                print(reader_support_arrays[channel].view(support_arrays[channel].dtype))
                print(support_arrays[channel])
                print('Test failed, original support array and support array from '
                      'file differ in channel {0}'.format(channel))
                sys.exit(1)

        print('Test passed')


if __name__ == '__main__':
    main()
