#pragma once
#include <algorithm>
#include <atomic>
#include <vcruntime.h>

namespace Aether
{

    template<typename T>
    class RingBuffer
    {
    public:
        RingBuffer(size_t capacity)
            :m_capacity(capacity)
        {
            m_data=new T[m_capacity];
        }
        ~RingBuffer()
        {
            delete[] m_data;
        }
        size_t Write(const T* data,size_t size)
        {
            size_t data_to_write=std::min(size,CalculateAvailableSize());
            size_t data_written=0;
            if(m_write_pos>=m_read_pos)
            {
                size_t write_size=std::min(data_to_write,m_capacity-m_write_pos);
                for(size_t i=0;i<write_size;i++)
                {
                    m_data[m_write_pos+i]=data[i];
                }
                m_write_pos+=write_size;
                data_written+=write_size;
                if(m_write_pos==m_capacity)
                {
                    m_write_pos=0;
                }
                if(data_written<data_to_write)
                {
                    for(size_t i=0;i<data_to_write-data_written;i++)
                    {
                        new(&m_data[i]) T(data[data_written+i]);
                    }
                    m_write_pos=data_to_write-data_written;
                    data_written=data_to_write;
                }
                return data_written;
            }
            else
            {
                for(size_t i=0;i<data_to_write;i++)
                {
                    new(&m_data[m_write_pos+i]) T(data[i]);
                }
                m_write_pos+=data_to_write;
                data_written=data_to_write;   
                return data_written;
            }
            
        }
        size_t Read(T* data,size_t size)
        {
            size_t data_to_read=std::min(size,CalculateSize());
            size_t data_read=0;
            if(m_read_pos>=m_write_pos)
            {
                size_t read_size=std::min(data_to_read,m_capacity-m_read_pos);

                for(size_t i=0;i<read_size;i++)
                {
                    new(&data[i]) T(m_data[m_read_pos+i]);
                    m_data[m_read_pos + i].~T();
                }
                m_read_pos+=read_size;
                data_read+=read_size;
                if(m_read_pos==m_capacity)
                {
                    m_read_pos=0;
                }
                if(data_read<data_to_read)
                {
                    for(size_t i=0;i<data_to_read-data_read;i++)
                    {
                        new(&data[data_read+i]) T(m_data[i]);
                        m_data[i].~T();
                    }
                    m_read_pos=data_to_read-data_read;
                    data_read=data_to_read;
                }
                return data_read;
            }
            else
            {
                for(size_t i=0;i<data_to_read;i++)
                {
                    new(&data[i]) T(m_data[m_read_pos+i]);
                    m_data[m_read_pos + i].~T();
                }
                m_read_pos+=data_to_read;
                data_read=data_to_read;
                return data_read;
            }
           
        }
        size_t GetCapacity() const
        {
            return m_capacity;
        }
        size_t GetWritePos() const
        {
            return m_write_pos;
        }
        size_t GetReadPos() const
        {
            return m_read_pos;
        }
        size_t CalculateSize()const
        {
            if(m_write_pos>=m_read_pos)
            {
                return m_write_pos-m_read_pos;
            }
            else
            {
                return m_capacity-m_read_pos+m_write_pos;
            }
        }
        size_t CalculateAvailableSize() const
        {
            if(m_write_pos>=m_read_pos)
            {
                return m_capacity-m_write_pos+m_read_pos-1;
            }
            else
            {
                return m_read_pos-m_write_pos-1;
            }
        }
        bool IsEmpty() const
        {
            return m_write_pos==m_read_pos;
        }
        bool IsFull() const
        {
            return CalculateAvailableSize()==0;
        }
    private:
        T* m_data;
        size_t m_capacity;
        size_t m_write_pos=0;
        size_t m_read_pos=0;
    };
}