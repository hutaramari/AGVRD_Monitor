#include "protocol.h"

Protocol::Protocol(QObject *parent) : QObject(parent)
{

}

quint32 Protocol::decode(QByteArray &bufferInArray, quint8 *bufferDecoded, quint8 *cmdDecoded)
{
    quint16 retsize = 0;
    quint16 checksize = 0;
    quint16 crccheck = 0;
    quint16 computecheck = 0;
    quint8  pkttype;
    quint16 pktsize;
    quint16 spotsnum = 0;
    quint8 start = 0;
    quint8 headerSize = 0;
    quint8 offset = 0;

    if(bufferInArray.size() < 128)    // COMMAND
    {
        if(mode_ == PROTOCOL_BEA)
        {
            while(bufferInArray.size() > PROTOCOL_BEA_HEADERSIZE)
            {
                if(bufferInArray.at(0) == 0x02 &&
                   bufferInArray.at(1) == 0x02 &&
                   bufferInArray.at(2) == PROTOCOL_BEA_HEADER_0 &&
                   bufferInArray.at(3) == PROTOCOL_BEA_HEADER_1 &&
                   bufferInArray.at(4) == PROTOCOL_BEA_HEADER_2 &&
                   bufferInArray.at(5) == PROTOCOL_BEA_HEADER_3)
                {
                    start = 1;
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }
        else
        {
            while(bufferInArray.size() > PROTOCOL_LEUZE_HEADERSIZE)
            {
                if(bufferInArray.at(0) == 0x02 &&
                   bufferInArray.at(1) == PROTOCOL_LEUZE_HEADER_0 &&
                   bufferInArray.at(2) == PROTOCOL_LEUZE_HEADER_1 &&
                   bufferInArray.at(3) == PROTOCOL_LEUZE_HEADER_2 &&
                   bufferInArray.at(4) == PROTOCOL_LEUZE_HEADER_3 &&
                   bufferInArray.at(5) == PROTOCOL_LEUZE_HEADER_4)
                {
                    start = 1;
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }

        if((start == 1) && (bufferInArray.size() >= 8))
        {
            checksize = static_cast<quint8>(bufferInArray.at(6)) * 256 + static_cast<quint8>(bufferInArray.at(7));
            bufferInArray.remove(0, 8);// Remove header and len parts

            if(bufferInArray.size() >= (checksize + 1))
            {
                memcpy(cmdDecoded, bufferInArray.data(), checksize);

                crccheck = static_cast<quint8>(bufferInArray.at(checksize));
                computecheck = beaComputeCHK(cmdDecoded, checksize);

                if(crccheck == computecheck)
                {
                    retsize = checksize;
                    packetType = AGV_MODE_CMD;
                }
                else
                {
                    qDebug("[AGV Public Command Protocol] error checksum!");
                    qDebug()<<crccheck<<" "<<computecheck<<" "<<checksize;
                    retsize = 0;
                }
            }
        }
    }
    else    // MDI
    {
        if(mode_ == PROTOCOL_BEA)
        {
            while(bufferInArray.size() > 4)
            {
                if(bufferInArray.at(0) == PROTOCOL_BEA_HEADER_0 &&
                   bufferInArray.at(1) == PROTOCOL_BEA_HEADER_1 &&
                   bufferInArray.at(2) == PROTOCOL_BEA_HEADER_2 &&
                   bufferInArray.at(3) == PROTOCOL_BEA_HEADER_3)
                {
                    start = 1;
                    headerSize = PROTOCOL_BEA_MDIHEADERSIZE;
                    offset = 4;
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }
        else
        {
            while(bufferInArray.size() > 5)
            {
                if(bufferInArray.at(0) == PROTOCOL_LEUZE_HEADER_0 &&
                   bufferInArray.at(1) == PROTOCOL_LEUZE_HEADER_1 &&
                   bufferInArray.at(2) == PROTOCOL_LEUZE_HEADER_2 &&
                   bufferInArray.at(3) == PROTOCOL_LEUZE_HEADER_3 &&
                   bufferInArray.at(4) == PROTOCOL_LEUZE_HEADER_4)
                {
                    start = 1;
                    headerSize = PROTOCOL_LEUZE_MDIHEADERSIZE;
                    offset = 5;
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }

        if((start == 1) && (bufferInArray.size() > headerSize))
        {
            // Packet type
            pkttype = static_cast<quint8>(bufferInArray.at(offset));
            // Packet size
            pktsize = static_cast<quint8>(bufferInArray.at(offset + 1)) * 256 + static_cast<quint8>(bufferInArray.at(offset + 2));
            // Scan points number in this frame
            spotsnum = static_cast<quint8>(bufferInArray.at(offset + 15)) * 256 + static_cast<quint8>(bufferInArray.at(offset + 16));
            switch (pkttype)
            {
            case 0: // Distance only
                checksize = headerSize + spotsnum * 2;
                break;
            case 1: // Distance + pulsewidth
                checksize = headerSize + spotsnum * 4;
                break;
            default: // >1?
                checksize = headerSize + spotsnum * 4;
                break;
            }
            // Check if the frame size is correct
            if(bufferInArray.size() >= (checksize + 2)) // 2 is size of CRC
            {
                // Copy data to outbuffer starting from packet type
                memcpy(bufferDecoded, bufferInArray.data(), checksize);

                // CRC check
                crccheck = static_cast<quint8>(bufferInArray.at(checksize)) * 256 + static_cast<quint8>(bufferInArray.at(checksize + 1));
                computecheck = beaComputeCRC(bufferDecoded, checksize);
                if(crccheck == computecheck)
                {
                    // Remove the SYNC part
                    checksize -= offset;
                    memmove(bufferDecoded, &bufferDecoded[offset], checksize);
                    retsize = checksize; // Size without SYNC
                }
                else
                {
                    qDebug("[1 BEA Public Protocol] error crc in MDI frame!");
                    retsize = 0;
                }
                packetType = AGV_MODE_MDI;
            }
        }
    }

    return retsize;

}

quint32 Protocol::decode(QByteArray &bufferInArray, quint8 bufferDecoded[], quint8 agvMode)
{
    quint16 retsize = 0;
    quint16 checksize = 0;
    quint16 crccheck;
    quint16 computecheck;
    quint8  pkttype;
    quint16 pktsize;
    quint16 spotsnum = 0;
    quint8 headerSize = 0;
    quint8 offset = 0;

    if(agvMode == AGV_MODE_CMD)    // Command
    {
        if(mode_ == PROTOCOL_BEA)
        {
            while(bufferInArray.size() > PROTOCOL_BEA_HEADERSIZE)
            {
                if(bufferInArray.at(0) == 0x02 &&
                   bufferInArray.at(1) == 0x02 &&
                   bufferInArray.at(2) == PROTOCOL_BEA_HEADER_0 &&
                   bufferInArray.at(3) == PROTOCOL_BEA_HEADER_1 &&
                   bufferInArray.at(4) == PROTOCOL_BEA_HEADER_2 &&
                   bufferInArray.at(5) == PROTOCOL_BEA_HEADER_3)
                {
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }
        else
        {
            while(bufferInArray.size() > PROTOCOL_LEUZE_HEADERSIZE)
            {
                if(bufferInArray.at(0) == 0x02 &&
                   bufferInArray.at(1) == PROTOCOL_LEUZE_HEADER_0 &&
                   bufferInArray.at(2) == PROTOCOL_LEUZE_HEADER_1 &&
                   bufferInArray.at(3) == PROTOCOL_LEUZE_HEADER_2 &&
                   bufferInArray.at(4) == PROTOCOL_LEUZE_HEADER_3 &&
                   bufferInArray.at(5) == PROTOCOL_LEUZE_HEADER_4)
                {
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }

        if(bufferInArray.size() >= 8)
        {
            checksize = static_cast<quint8>(bufferInArray.at(6)) * 256 + static_cast<quint8>(bufferInArray.at(7));
            bufferInArray.remove(0, 8);// Remove header and len parts

            if(bufferInArray.size() >= (checksize + 1))
            {
                memcpy(bufferDecoded, bufferInArray.data(), checksize);

                crccheck = static_cast<quint8>(bufferInArray.at(checksize));
                computecheck = beaComputeCHK(bufferDecoded, checksize);

                if(crccheck == computecheck)
                {
                    retsize = checksize;
                }
                else
                {
                    qDebug("[AGV Public Command Protocol] error checksum!");
                    qDebug()<<crccheck<<" "<<computecheck<<" "<<checksize;
                    retsize = 0;
                }
            }
        }
    }
    else if(agvMode == AGV_MODE_MDI)   // MDI
    {
        if(mode_ == PROTOCOL_BEA)
        {
            while(bufferInArray.size() > 4)
            {
                if(bufferInArray.at(0) == PROTOCOL_BEA_HEADER_0 &&
                   bufferInArray.at(1) == PROTOCOL_BEA_HEADER_1 &&
                   bufferInArray.at(2) == PROTOCOL_BEA_HEADER_2 &&
                   bufferInArray.at(3) == PROTOCOL_BEA_HEADER_3)
                {
                    headerSize = PROTOCOL_BEA_MDIHEADERSIZE;
                    offset = 4;
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }
        else
        {
            while(bufferInArray.size() > 5)
            {
                if(bufferInArray.at(0) == PROTOCOL_LEUZE_HEADER_0 &&
                   bufferInArray.at(1) == PROTOCOL_LEUZE_HEADER_1 &&
                   bufferInArray.at(2) == PROTOCOL_LEUZE_HEADER_2 &&
                   bufferInArray.at(3) == PROTOCOL_LEUZE_HEADER_3 &&
                   bufferInArray.at(4) == PROTOCOL_LEUZE_HEADER_4)
                {
                    headerSize = PROTOCOL_LEUZE_MDIHEADERSIZE;
                    offset = 5;
                    break;
                }
                else
                {
                    bufferInArray.remove(0,1);
                }
            }
        }


        if(bufferInArray.size() >= headerSize)
        {
            // Packet type
            pkttype = static_cast<quint8>(bufferInArray.at(offset));
            // Packet size
            pktsize = static_cast<quint8>(bufferInArray.at(offset + 1)) * 256 + static_cast<quint8>(bufferInArray.at(offset + 2));
            // Scan points number in this frame
            spotsnum = static_cast<quint8>(bufferInArray.at(offset + 15)) * 256 + static_cast<quint8>(bufferInArray.at(offset + 16));
            switch (pkttype)
            {
            case 0: // Distance only
                checksize = headerSize + spotsnum * 2;
                break;
            case 1: // Distance + pulsewidth
                checksize = headerSize + spotsnum * 4;
                break;
            default: // >1?
                checksize = headerSize + spotsnum * 4;
                break;
            }
            // Check if the frame size is correct
            if(bufferInArray.size() == (checksize + 2)) // 2 is size of CRC
            {
                // Copy data to outbuffer starting from packet type
                memcpy(bufferDecoded, bufferInArray.data(), checksize);

                // CRC check
                crccheck = static_cast<quint8>(bufferInArray.at(checksize)) * 256 + static_cast<quint8>(bufferInArray.at(checksize + 1));
                computecheck = beaComputeCRC(bufferDecoded, checksize);
                if(crccheck == computecheck)
                {
                    // Remove the SYNC part
                    checksize -= offset;
                    memmove(bufferDecoded, &bufferDecoded[offset], checksize);
                    retsize = checksize; // Size without SYNC
                }
                else
                {
                    qDebug("[2 BEA Public Protocol] error crc in MDI frame!");
                    retsize = 0;
                }
            }
        }
    }

    return retsize;
}

quint16 Protocol::beaComputeCRC(quint8 *msgBuffer, quint16 msgSize)
{
    quint16 crc = 0;
    quint16 i, j;

    for (i = 0u; i < msgSize; i++) /* size_ = number of protocol bytes without CRC */
    {
        crc ^= static_cast<quint16>(msgBuffer[i]<<8);

        for(j=0;j<8;j++)
        {
            if((crc & 0x8000) !=0)
            {
                crc = static_cast<quint16>((crc<<1)^BEA_POLYNOM);
            }
            else
            {
                crc <<=1;
            }
        }
    }
    return crc;
}

quint8 Protocol::beaComputeCHK(quint8 *msgBuffer, quint16 msgSize)
{
    quint8 chk=0;
    quint16 i;

    for (i=0;i<msgSize;i++)
    {
        chk ^= msgBuffer[i];
    }

    return chk;
}

quint16 Protocol::encode(quint8 buffer[], quint8 bufferEncoded[], quint16 bufSize)
{
    quint32 cpycnt;
    quint16 crc;
    quint16 retsize = 0u;

    retsize = bufSize + static_cast<quint16>(PROTOCOL_BEA_HEADERSIZE);

    bufferEncoded[0] = static_cast<quint8>(PROTOCOL_BEA_HEADER_0);
    bufferEncoded[1] = static_cast<quint8>(PROTOCOL_BEA_HEADER_1);
    bufferEncoded[2] = static_cast<quint8>(PROTOCOL_BEA_HEADER_2);
    bufferEncoded[3] = static_cast<quint8>(PROTOCOL_BEA_HEADER_3);
    bufferEncoded[4] = static_cast<quint8>(PROTOCOL_BEA_VERSION);

    /* buffer5 and 6 is the real size. 7 and 8 acturally is always 0 */
    bufferEncoded[5] = lobyte(retsize + PROTOCOL_BEA_FOOTERSIZE);
    bufferEncoded[6] = hibyte(retsize + PROTOCOL_BEA_FOOTERSIZE);

    bufferEncoded[7] = 2;
    bufferEncoded[8] = 0;
    bufferEncoded[9] = 0;
    bufferEncoded[10] = 0;

    for(cpycnt = 0; cpycnt < bufSize; cpycnt++)
    {
        bufferEncoded[PROTOCOL_BEA_HEADERSIZE + cpycnt] = buffer[cpycnt];
    }

    crc = beaComputeCRC(bufferEncoded, retsize);

    bufferEncoded[retsize] = lobyte(crc);
    bufferEncoded[retsize+1] = hibyte(crc);

    retsize += PROTOCOL_BEA_FOOTERSIZE;

    return retsize;
}

quint16 Protocol::encode(quint8 buffer[], quint8 bufferEncoded[], quint16 bufSize, quint8 agvMode)
{
    quint16 ret = 0;
    quint16 crc;
    quint16 offset;

    if(agvMode == AGV_MODE_CMD)
    {
        if(mode_ == PROTOCOL_BEA)
        {
            offset = 0;
            bufferEncoded[offset++] = 0x02;
            bufferEncoded[offset++] = 0x02;
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_BEA_HEADER_0);
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_BEA_HEADER_1);
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_BEA_HEADER_2);
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_BEA_HEADER_3);
        }
        else
        {
            offset = 0;
            bufferEncoded[offset++] = 0x02;
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_LEUZE_HEADER_0);
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_LEUZE_HEADER_1);
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_LEUZE_HEADER_2);
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_LEUZE_HEADER_3);
            bufferEncoded[offset++] = static_cast<quint8>(PROTOCOL_LEUZE_HEADER_4);
        }
        bufferEncoded[offset++] = hibyte(bufSize);
        bufferEncoded[offset++] = lobyte(bufSize);

        memcpy(&bufferEncoded[offset], buffer, bufSize);

        crc = beaComputeCHK(&bufferEncoded[offset], bufSize);
        offset += bufSize;
        bufferEncoded[offset++] = lobyte(crc);

        ret = offset;
    }

    return ret;
}

void Protocol::packCMD(quint16 cmd, quint8 *buffer, quint16 *cmdSize)
{
    switch (cmd) {
    case 49396:
    case 50004:
        buffer[0] = lobyte(cmd);
        buffer[1] = hibyte(cmd);
        *cmdSize = 2;
        break;

    case 50011:
        buffer[0] = lobyte(cmd);
        buffer[1] = hibyte(cmd);
        buffer[2] = 0;
        *cmdSize = 3;
        break;

    case 50005:
        buffer[0] = lobyte(cmd);
        buffer[1] = hibyte(cmd);
        *cmdSize = 2;
        break;

    default:
        break;
    }
}

void Protocol::packCMD(QString cmd, quint8 *buffer, quint16 *cmdSize)
{
    quint16 offset;
    QByteArray cmd_ba;

    offset = 0;
    cmd_ba = cmd.toUtf8();
    for(int i = 0; i < cmd.size(); i++)
    {
        buffer[i] = static_cast<quint8>(cmd_ba.at(i));
        offset++;
    }
    *cmdSize = offset;
}

quint8 Protocol::lobyte(quint16 w)
{
    return static_cast<quint8>(w & 0x00FFu);
}

quint8 Protocol::hibyte(quint16 w)
{
    return static_cast<quint8>((w & 0xFF00u) >> 8);
}

void Protocol::setMode(ProtocolType mode)
{
    mode_ = mode;
}
