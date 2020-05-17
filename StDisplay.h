#ifndef STDISPLAY_H
#define STDISPLAY_H

/*
* Copyright 2020 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the Smalltalk parser/compiler library.
*
* The following is the license that applies to this copy of the
* library. For a license to use the library under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

#include <QWidget>


namespace St
{
    class Bitmap
    {
    public:
        enum { PixPerByte = 8, PixPerWord = PixPerByte * 2 };
        Bitmap():d_buf(0),d_wordLen(0) {}
        Bitmap( quint8* buf, qint16 wordLen, qint16 pixWidth, qint16 pixHeight );
        inline const quint8* scanLine(int y) const
        {
            return d_buf + ( y * d_pixLineWidth / PixPerByte );
        }
        inline bool test(quint16 x, quint16 y) const
        {
            const int bytePos = ( y * d_pixLineWidth + x ) / PixPerByte;
            const quint8 bitpos = 7 - x % 8 ;
            const quint8 pat = 1 << bitpos;
            Q_ASSERT( bytePos < d_wordLen * 2 );
            return ( d_buf[bytePos] & pat ) > 0;
        }
        inline void set(quint16 x, quint16 y, bool on) const
        {
            const int bytePos = ( y * d_pixLineWidth + x ) / PixPerByte;
            const quint8 bitpos = 7 - x % 8 ;
            const quint8 pat = 1 << bitpos;
            Q_ASSERT( bytePos < d_wordLen * 2 );
            if( on )
                d_buf[bytePos] |= pat;
            else
                d_buf[bytePos] &= ~pat;
        }
        qint16 lineWidth() const { return d_pixLineWidth; }
        qint16 width() const { return d_pixWidth; }
        qint16 height() const { return d_pixHeight; }
        qint16 wordAt(qint16 i ) const;
        void wordAtPut( qint16 i, qint16 v );
        bool isNull() const { return d_buf == 0; }
        QImage toImage() const;
        QImage toImage(quint16 x, quint16 y, quint16 w, quint16 h) const;
    private:
        qint16 d_pixWidth, d_pixHeight, d_pixLineWidth, d_wordLen;
        quint8* d_buf;
    };

    class Display : public QWidget
    {
        Q_OBJECT
    public:
        explicit Display(QWidget *parent = 0);
        static Display* inst();
        static bool s_run;

        void setBitmap( const Bitmap& );
        void setCursorBitmap( const Bitmap& );
    protected:
        void paintEvent(QPaintEvent *);
        void timerEvent(QTimerEvent *);
        void closeEvent(QCloseEvent * event);
        QString renderTitle() const;
    private:
        Bitmap d_bitmap;
        QImage d_img;
    };

    class BitBlt
    {
    public:
        // This is a textbook implementation according to Blue Book chapter 18 "Simulation of BitBlt".
        // Known to be inefficient; focus is on functionality and compliance.
        struct Input
        {
            const Bitmap* sourceBits;
            Bitmap* destBits;
            const Bitmap* halftoneBits;
            qint16 combinationRule;
            qint16 destX, clipX, clipWidth, sourceX, width;
            qint16 destY, clipY, clipHeight, sourceY, height;
            Input():sourceBits(0),destBits(0),halftoneBits(0),combinationRule(0),
                destX(0),clipX(0),clipWidth(0),sourceX(0),width(0),
                destY(0),clipY(0),clipHeight(0),sourceY(0),height(0) {}
        };
        BitBlt( const Input& );
        void copyBits();
    protected:
        void clipRange();
        void computeMasks();
        void checkOverlap();
        void calculateOffsets();
        void copyLoop();
        inline qint16 merge(qint16 source, qint16 destination );
    private:
        const Bitmap* sourceBits;
        const Bitmap* halftoneBits;
        const qint16 destX, clipX, clipWidth, sourceX, width; // pixel
        const qint16 destY, clipY, clipHeight, sourceY, height; // pixel
        const qint16 combinationRule;
        Bitmap* destBits;
        qint16 sourceRaster;
        qint16 destRaster;
        qint16 skew, mask1, mask2, skewMask, nWords, vDir, hDir;
        qint16 sx, sy, dx, dy, w, h; // pixel
        qint16 sourceIndex, destIndex, sourceDelta, destDelta;
        bool preload;
        static const QList<qint16> RightMasks;
        static const qint16 AllOnes;
    };
}

#endif // STDISPLAY_H
