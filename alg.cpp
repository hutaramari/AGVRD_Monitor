#include "alg.h"

Alg::Alg()
{
    theta_f = 8.90f;
    angleIndexBegin_f = 0.0f;
    angleIndexBegin_w = 0;

    stepSector = 2;
    widthSector = 11; // Not used
    widthWindow = 278 - 42; // Not used
    numSector = TSKWIN_SECTOR_NUM; // (309-42-42)/2+1

    totalLzrRx = RX_WINDOW_ROWS*RX_WINDOW_COLS*1000;
    totalLzrTx = TX_WINDOW_ROWS*TX_WINDOW_COLS*1000;

    counter = 0;

    HPrx_Idx = PPrx_Idx = HPtx_Idx = PPtx_Idx = 0;
    HPrx_MaxVal = PPrx_MaxVal = HPtx_MaxVal = PPtx_MaxVal = 0;

    memset(&tskWin_rx_sector_s.sector_wa[0][0], 0, sizeof (tskWin_sector_t));
    memset(&tskWin_tx_sector_s.sector_wa[0][0], 0, sizeof (tskWin_sector_t));
    memset(&tskWin_temp_sector_s.sector_wa[0][0], 0, sizeof (tskWin_sector_t));
}

int Alg::dilation(quint16 *bufferIn_, quint16 *bufferOut_, int size_)
{
    int ret = -1;
    int n, i;
    quint16 left, middle, right, max;

    if(size_ == ISR_MAXWINDOWLEDS * ISR_WINDOWSAMPLES / 2)
    {
        for (n = 0; n < ISR_MAXWINDOWLEDS; n++) // NOTE: substract 2 ref leds
        {
            for (i = 0; i < ISR_WINDOWSAMPLES/2; i++)
            {
                left = middle = right = max = 0;

                switch (i)
                {
                case 0:
                    middle = bufferIn_[n * ISR_WINDOWSAMPLES/2 + 0];
                    right  = bufferIn_[n * ISR_WINDOWSAMPLES/2 + i + 1];
                    break;
                case (ISR_WINDOWSAMPLES/2 - 1):
                    middle = bufferIn_[n * ISR_WINDOWSAMPLES/2 + i];
                    left   = bufferIn_[n * ISR_WINDOWSAMPLES/2 + i - 1];
                    break;
                default:
                    left   = bufferIn_[n * ISR_WINDOWSAMPLES/2 + i - 1];
                    middle = bufferIn_[n * ISR_WINDOWSAMPLES/2 + i];
                    right  = bufferIn_[n * ISR_WINDOWSAMPLES/2 + i + 1];
                    break;
                }

                if(middle > max)
                {
                    max = middle;
                }
                if(left > max)
                {
                    max = left;
                }
                if(right > max)
                {
                    max = right;
                }

                bufferOut_[n * ISR_WINDOWSAMPLES/2 + i] = max;
            }
#ifdef TSKWIN_DEBUGWMSREQ
            for (i = 0; i < ISR_WINDOWSAMPLES/2; i++)
            {
                bufferOut_[n*ISR_WINDOWSAMPLES/2+i] = 0;
            }
#endif
        }
#ifdef TSKWIN_DEBUGWMSREQ
        bufferOut_[68] = 395u;
        bufferOut_[69] = 395u;
        bufferOut_[70] = 395u;
        bufferOut_[71] = 216u;
        bufferOut_[72] = 57u;

        bufferOut_[88] = 6u;
        bufferOut_[89] = 58u;
        bufferOut_[90] = 181u;
        bufferOut_[91] = 213u;
        bufferOut_[92] = 213u;
        bufferOut_[93] = 222u;
        bufferOut_[94] = 222u;
        bufferOut_[95] = 222u;
        bufferOut_[96] = 214u;
        bufferOut_[97] = 88u;
        bufferOut_[98] = 7u;

        bufferOut_[113] = 3u;
        bufferOut_[114] = 52u;
        bufferOut_[115] = 200u;
        bufferOut_[116] = 407u;
        bufferOut_[117] = 407u;
        bufferOut_[118] = 407u;
#endif
        ret = 0;
    }

    return ret;
}

int Alg::spatialDiscri(quint16 *bufferIn_, quint16 *bufferOut_, int size_)
{
    int ret = -1;
    int n, i;
    quint16 temp;
    quint16 idx, line, col;

    if(size_ == ISR_MAXWINDOWLEDS * ISR_WINDOWSAMPLES / 2)
    {
        // setWindow
        for(n = 0; n < TSKIN_SPATIALDISCRILINES; n++)
        {
            for(i = 0; i < TSKIN_SPATIALDISCRICOLUMNS; i++)
            {
                bufferOut_[n * TSKIN_SPATIALDISCRICOLUMNS + i] = 1000;
            }
        }
        // mask
        for(n = 0; n < ISR_MAXWINDOWLEDS; n++) // NOTE: substract 2 ref leds
        {
            angleIndexBegin_f = theta_f * static_cast<float>(n);
            angleIndexBegin_w = static_cast<quint16>(qRound(angleIndexBegin_f));

            for(i = 0; i < ISR_WINDOWSAMPLES / 2; i++)
            {
                for(idx = 0; idx < windowSpatialValues_s[i].indexNum_w; idx++)
                {
                    temp = windowSpatialValues_s[i].indexValues_wa[idx] + angleIndexBegin_w;
                    line = temp / TSKIN_SPATIALDISCRICOLUMNS;
                    col = temp % TSKIN_SPATIALDISCRICOLUMNS;
                    if(bufferIn_[n * ISR_WINDOWSAMPLES / 2 + i] < bufferOut_[line * TSKIN_SPATIALDISCRICOLUMNS + col])
                    {
                        bufferOut_[line * TSKIN_SPATIALDISCRICOLUMNS + col] = bufferIn_[n * ISR_WINDOWSAMPLES / 2 + i];
                    }
                }
            }
        }

        ret = 0;
    }

    return  ret;
}

void Alg::lzrAttenuation(quint16 bufferIn_[], int size_)
{
    indx_cTx = 38;
    indx_rTx = 21;

    indx_cRx = 37;
    indx_rRx = 4;

    // qDebug()<<"sizeof RX sector is "<<sizeof (RX_sector); // result is 8
    for(int i = 0; i < numSector; i++)
    {
        if(i == 8)
        {
            counter++;
        }
        // =======================================================================
        // RX sector
        // =======================================================================
        pollutionMapping(&tskWin_rx_sector_s, bufferIn_, size_, indx_rRx, indx_cRx, RX_WINDOW_ROWS, RX_WINDOW_COLS);
        // Homogeneous pollution
        for(int j = 0; j < RX_WINDOW_ROWS; j++)
        {
            for(int k = 0; k < RX_WINDOW_COLS; k++)
            {
                TempBuf[j*RX_WINDOW_COLS+k] = tskWin_rx_sector_s.sector_wa[j][k];
            }
        }
        LZR_HPrx[i] = median(TempBuf, RX_WINDOW_ROWS*RX_WINDOW_COLS);

        // Punctual pollution
        removeOffset(&tskWin_rx_sector_s, LZR_HPrx[i], RX_WINDOW_ROWS, RX_WINDOW_COLS);
        imerode(&tskWin_rx_sector_s, RX_WINDOW_ROWS, RX_WINDOW_COLS, 3);
        createMask(&tskWin_rx_sector_s, RX_WINDOW_ROWS, RX_WINDOW_COLS);
        LZR_PPrx[i] = calcAttenuation(&tskWin_rx_sector_s, totalLzrRx, RX_WINDOW_ROWS, RX_WINDOW_COLS, RX_WINDOW_PP_PERCENT);

        // =======================================================================
        // TX sector
        // =======================================================================
        pollutionMapping(&tskWin_tx_sector_s, bufferIn_, size_, indx_rTx, indx_cTx, TX_WINDOW_ROWS, TX_WINDOW_COLS);
        // Homogeneous pollution
        for(int j = 0; j < TX_WINDOW_ROWS; j++)
        {
            for(int k = 0; k < TX_WINDOW_COLS; k++)
            {
                TempBuf[j*TX_WINDOW_COLS+k] = tskWin_tx_sector_s.sector_wa[j][k];
            }
        }
        LZR_HPtx[i] = median(TempBuf, TX_WINDOW_ROWS*TX_WINDOW_COLS);

        // Punctual pollution
        removeOffset(&tskWin_tx_sector_s, LZR_HPtx[i], TX_WINDOW_ROWS, TX_WINDOW_COLS);
        imerode(&tskWin_tx_sector_s, TX_WINDOW_ROWS, TX_WINDOW_COLS, 2);
        createMask(&tskWin_tx_sector_s, TX_WINDOW_ROWS, TX_WINDOW_COLS);
        LZR_PPtx[i] = calcAttenuation(&tskWin_tx_sector_s, totalLzrTx, TX_WINDOW_ROWS, TX_WINDOW_COLS, TX_WINDOW_PP_PERCENT);

        // =======================================================================
        // Update step
        // =======================================================================
        indx_cTx += stepSector;
        indx_cRx += stepSector;
    }

    // =======================================================================
    // Calc Max value
    // =======================================================================
    HPrx_Idx = indexMax(LZR_HPrx, numSector, &HPrx_MaxVal);
    PPrx_Idx = indexMax(LZR_PPrx, numSector, &PPrx_MaxVal);
    HPtx_Idx = indexMax(LZR_HPtx, numSector, &HPtx_MaxVal);
    PPtx_Idx = indexMax(LZR_PPtx, numSector, &PPtx_MaxVal);

    // set HP percentage
    HPrx_MaxVal /= 10;
    HPtx_MaxVal /= 10;

}

quint16 Alg::indexMax(quint16 buf_[], int size_, quint16 *max_)
{
    int ret;

    *max_ = buf_[0];
    ret = 0;

    for (int i = 1; i < size_; i++)
    {
        if(buf_[i] > *max_)
        {
            *max_ = buf_[i];
            ret = i;
        }
    }

    return static_cast<quint16>(ret);
}
void Alg::sort(quint16 *buf_, int size_)
{
    while(size_ > 1)
    {
        for(int i = 0; i < size_-1; ++i)
        {
            if(buf_[i] > buf_[i+1])
            {
                quint16 temp = buf_[i];
                buf_[i] = buf_[i+1];
                buf_[i+1] = temp;
            }
        }
        size_--;
    }
}

quint16 Alg::median(quint16 *buf_, int size_)
{
    quint16 result;    
    int half;

    half = size_/2;
    sort(buf_, size_);
    result = buf_[half];

    return result;
}

void Alg::pollutionMapping(tskWin_sector_t *sector_, quint16 bufIn[], int size_, int rbegin, int cbegin, int rlen, int clen)
{
    int line, col, indx;

    for(int j = 0; j < rlen; j++)
    {
        for (int k = 0; k < clen; k++)
        {
            line = rbegin + j;
            col = cbegin + k;
            indx = line * TSKIN_SPATIALDISCRICOLUMNS + col;
            sector_->sector_wa[j][k] = bufIn[indx];
        }
    }
}

void Alg::removeOffset(tskWin_sector_t *sector_, quint16 offset_, int rsize_, int csize_)
{
    for(int i = 0; i < rsize_; ++i)
    {
        for(int j = 0; j < csize_; ++j)
        {
            if(sector_->sector_wa[i][j] < offset_)
            {
               sector_->sector_wa[i][j] = 0;
            }
            else
            {
                sector_->sector_wa[i][j] -= offset_;
            }
        }
    }
}

void Alg::imerode(tskWin_sector_t *sector_, int rsize_, int csize_, int factor_)
{
    Q_ASSERT(factor_ == 2 || factor_ == 3);

    quint16 minvalue;

    switch (factor_) {
    case 3:
        for (int i = 0; i < rsize_; i++)
        {
            for(int j = 0; j < csize_; j++)
            {
                minvalue = 1000;
                for(int n = 0; n < 3; n++)
                {
                    for(int m = 0; m < 3; m++)
                    {
                        int rowPos = i - 1 + n;
                        int colPos = j - 1 + m;
                        if(rowPos < 0) rowPos = 0;
                        if(rowPos >= rsize_) rowPos = rsize_ - 1;
                        if(colPos < 0) colPos = 0;
                        if(colPos >= csize_) colPos = csize_ - 1;
                        if(sector_->sector_wa[rowPos][colPos] < minvalue) minvalue = sector_->sector_wa[rowPos][colPos];
                    }
                }
                // set cental point
                tskWin_temp_sector_s.sector_wa[i][j] = minvalue;
            }
        }
        break;
    default:
        for(int i = 0; i < rsize_; ++i)
        {
            for(int j = 0; j < csize_; ++j)
            {
                minvalue = 1000;
                for(int n = 0; n < 2; n++)
                {
                    for(int m = 0; m < 2; m++)
                    {
                        int rowPos = i + n;
                        int colPos = j + m;
                        if(rowPos >= rsize_) rowPos = rsize_ - 1;
                        if(colPos >= csize_) colPos = csize_ - 1;
                        if(sector_->sector_wa[rowPos][colPos] < minvalue) minvalue = sector_->sector_wa[rowPos][colPos];
                    }
                }
                //ret[i][j] = minvalue;
                tskWin_temp_sector_s.sector_wa[i][j] = minvalue;
            }
        }
        break;
    }

    memcpy(sector_, &tskWin_temp_sector_s.sector_wa[0][0], sizeof (tskWin_sector_t));
}

void Alg::createMask(tskWin_sector_t *sector_, int rsize_, int csize_)
{
    for(int i = 0; i < rsize_; i++)
    {
        for(int j = 0; j < csize_; j++)
        {
            if(sector_->sector_wa[i][j] > 0)
            {
                sector_->sector_wa[i][j] = 1;
            }
        }
    }
}

quint16 Alg::calcAttenuation(tskWin_sector_t *sector_, int tot_, int rsize_, int csize_, int percent_)
{
    int sum = 0;

    for(int i = 0; i < rsize_; i++)
    {
        for(int j = 0; j < csize_; j++)
        {
            if(sector_->sector_wa[i][j] == 1)
            {
                sum += 1000;// Need to modification!!!!!
            }
        }
    }

    Q_ASSERT(tot_ != 0);
    sum *= percent_;
    sum /= tot_;

    return static_cast<quint16>(sum);
}
