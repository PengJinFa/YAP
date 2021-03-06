/*
#include "SampleDataProtocol.h"
#include <QByteArray>
#include <QString>
#include <cassert>

#define PACK_ERROR_RETURN_FALSE(a)\
	if(mpeSuccess != (a))\
{\
	return false;\
}

//卡勒幅谱仪数据标识和结构数据之间有一些结构信息，这里直接跳过这些字段。


const int StartPadding = 92; //
const int DataPadding  = 52;
const int EndPadding = 20;

bool SampleDataStart::Pack( QByteArray &byteArray )
{
    //检测填充字节数。
    int x = sizeof(uint32_t) + sizeof(HeadItem) * 11;
    int y = sizeof(uint32_t) + sizeof(HeadItem) * 6;
    int z = sizeof(uint32_t) + sizeof(HeadItem) * 2;
    assert( StartPadding == x && DataPadding == y && EndPadding == z);
    //

    byteArray = QByteArray::fromRawData((char*)this, sizeof(SampleDataStart));
    return true;
}

bool SampleDataStart::Unpack( QByteArray &byteArray )
{
    if(byteArray.length() != sizeof(SampleDataStart))
    {
        return false;
    }
    else
    {
        memcpy(this, byteArray.data(), byteArray.length());
    }

	return true;
}



bool SampleDataData::Pack( QByteArray &byteArray )
{
    //uint32_t cmd_id;
    //uint32_t rp_id;
    //uint32_t dim23456_index;
    //uint32_t rec;
    //float coeff;
    //std::vector<std::complex<float>> data;

    QByteArray data1 = QByteArray::fromRawData((char*)(&cmd_id), sizeof(uint32_t));
    QByteArray data2 = QByteArray::fromRawData((char*)(&rp_id), sizeof(uint32_t));
    QByteArray data3 = QByteArray::fromRawData((char*)(&dim23456_index), sizeof(uint32_t));
    QByteArray data4 = QByteArray::fromRawData((char*)(&rec), sizeof(uint32_t));
    QByteArray data5 = QByteArray::fromRawData((char*)(&coeff), sizeof(float));

    QByteArray data6 = QByteArray::fromRawData((char*)(data.data()),sizeof(std::complex<float>)* data.size());
    byteArray = data1 + data2 + data3 + data4 + data5 + data6;

    return true;

}

bool SampleDataData::Unpack( QByteArray &byteArray )
{
    int byteIndex = 0;
    int len = byteArray.length();
    //1,
    this->cmd_id = *(uint32_t*)(byteArray.data() + byteIndex);
    byteIndex += sizeof(uint32_t);
    //2,
    this->rp_id = *(uint32_t*)(byteArray.data() + byteIndex);
    byteIndex += sizeof(uint32_t);
    //3,
    this->dim23456_index = *(uint32_t*)(byteArray.data() + byteIndex);
    byteIndex += sizeof(uint32_t);
    //4
    this->rec = *(uint32_t*)(byteArray.data() + byteIndex);
    byteIndex += sizeof(uint32_t);
    //5,
    this->coeff = *(float*)(byteArray.data() + byteIndex);
    byteIndex += sizeof(float);

    //6,计算vector的元素数目
    int count = (len - byteIndex)/ sizeof(std::complex<float>);
    this->data.resize(count);

    if( (len - byteIndex) % sizeof(std::complex<float>) != 0)
    {
        return false;
    }
    else
    {
        memcpy(this->data.data(), (char*)byteArray.data() + byteIndex, byteArray.length()-byteIndex);
        return true;
    }

}

void SampleDataData::CreatDemoData(double phi0, double freq, double dw, unsigned int dataCount)

{
    const double PI = 3.1415926;

    data.resize(dataCount);

    for(unsigned int i = 0; i < data.size(); i ++)
    {

        double temp = data.size() + 10 - i;
        double phi = 2* PI * freq * dw * i;

        data[i] = temp* sin(phi0 + phi);

        data[i].real(temp * cos(phi0 + phi));
        data[i].imag(temp * sin(phi0 + phi));
    }
}

bool SampleDataEnd::Pack( QByteArray &byteArray )
{
    byteArray = QByteArray::fromRawData((char*)this, sizeof(SampleDataEnd));
    return true;
}

bool SampleDataEnd::Unpack( QByteArray &byteArray )
{
    if(byteArray.length() != sizeof(SampleDataEnd))
    {
        return false;
    }
    else
    {
        memcpy(this, byteArray.data(), byteArray.length());
    }

    return true;
}




bool FloatArray::Pack(QByteArray &byteArray)
{

    QByteArray data1 = QByteArray::fromRawData((char*)(&flag), sizeof(int));

    QByteArray data2 = QByteArray::fromRawData((char*)(data.data()),sizeof(float)* data.size());
    byteArray = data1 + data2;
    return true;

}

bool FloatArray::Unpack(QByteArray &byteArray)
{
    //
    int byteIndex = 0;

    int len = byteArray.length();
    this->flag = *(int*)(byteArray.data() + byteIndex);
    byteIndex += sizeof(this->flag);

    int count = (len - byteIndex)/ sizeof(float);

    this->data.resize(count);

    if( (len - byteIndex) % sizeof(float) != 0)
    {
        return false;
    }
    else
    {
        memcpy(this->data.data(), (char*)byteArray.data() + byteIndex, byteArray.length()-byteIndex);
        return true;
    }


}


void FloatArray::CreateDemoStruct()
{

    data.resize(98);
    for(unsigned int i = 0; i < data.size(); i ++)
    {
        data[i] = float(i)* 1.0;
    }


}

void FloatArray::CreateDemoSinStruct(double pha0, double freq, double dw, unsigned int dataCount)
//pha0
//dw in unit of s.
{

    const double PI = 3.1415926;

    data.resize(dataCount);

    for(unsigned int i = 0; i < data.size(); i ++)
    {

        double temp = data.size() + 10 - i;

        double pha = 2* PI * freq * dw * i;
        data[i] = temp* sin(pha0 + pha);


    }

    //std::vector<std::complex<float>> data_vector2(314);
    //for(unsigned int i = 0; i < data_vector2.size(); i ++)
    //{
    //    double temp = data_vector2.size() + 10 - i;
    //    data_vector2[i].real( temp* 10* sin(i * dw + 30 * PI/ 180) );
    //    data_vector2[i].imag( temp* 10* cos(i * dw + 30 * PI/ 180) );
    //}

    //
}

*/
