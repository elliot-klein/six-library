/* =========================================================================
 * This file is part of io-c++ 
 * =========================================================================
 * 
 * (C) Copyright 2004 - 2014, MDA Information Systems LLC
 *
 * io-c++ is free software; you can redistribute it and/or modify
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

#ifndef __DATA_STREAM_H__
#define __DATA_STREAM_H__

#include "io/StringStream.h"
#include "io/Serializable.h"

/*!
 *  \file DataStream.h
 *  \brief  Wrap a StringStream with a Serializable interface.
 *
 *  This class is actually hiding an internal StringStream, but it
 *  is also considered to be Serializable, meaning that it conforms to
 *  the interface for a SerializableConnection
 */
namespace io
{
/*!
 *  \class DataStream
 *  \brief Wraps a StringStream with a Serializable interface.
 *
 *  This class is actually hiding an internal StringStream, but it
 *  is also considered to be Serializable, meaning that it conforms to
 *  the interface for a SerializableConnection
 */
class DataStream: public io::Serializable
{
public:
    //! Default constructor
    DataStream()
    {
    }
    //! Deconstructor
    virtual ~DataStream()
    {
    }

    //! Returns the number of bytes available to read.
    virtual sys::Off_T available()
    {
        return mStringStream.available();
    }

    /*!
     *  Read bytes from our byte stream into the buffer
     *  \param data The data buffer to read to
     *  \param size The size of the data buffer to read
     */
    virtual sys::SSize_T read(sys::byte* data, sys::Size_T size)
    {
        return mStringStream.read(data, size);
    }

    /*!
     *  Write bytes from into our byte stream from the buffer
     *  \param data The data buffer to read from
     *  \param size The size of the data buffer.
     */
    virtual void write(const sys::byte* data, sys::Size_T size)
    {
        mStringStream.write(data, size);
    }

    /*!
     *  Outputs this object into an output stream.
     *  \param os the OutputStream to write to
     */
    virtual void serialize(io::OutputStream& os)
    {
        mStringStream.streamTo(os);
    }

    /*!
     * Unpack this input stream to the object
     * \param is Stream to read object from
     */
    virtual void deserialize(io::InputStream& is)
    {
        is.streamTo(mStringStream);
    }

    io::StringStream& getStream()
    {
        return mStringStream;
    }
    const io::StringStream& getStream() const
    {
        return mStringStream;
    }
protected:
    io::StringStream mStringStream;
};
}

#endif
