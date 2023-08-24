#include <Windows.h>
#include <gdiplus.h>
#include <vector>
#include "PictureFrame.h"
#include "util.h"

void FloodFill(Gdiplus::Bitmap* bmp, Gdiplus::Point pt);

PictureFrame::PictureFrame() : _source(nullptr), _tmp(nullptr)
{
}

PictureFrame::~PictureFrame()
{
	_frameRects.clear();
	size_t size = _frameData.size();
	for (size_t i = 0; i < size; ++i)
	{
		delete _frameData[i];
	}
	_frameData.clear();
}

size_t PictureFrame::framesNum() const
{
	return _frameData.size();
}

Gdiplus::Region* PictureFrame::getFrameDataAt(UINT index) const
{
	return _frameData[index];
}

// �ʐ^�t���[��������
void PictureFrame::search(Gdiplus::Bitmap* bitmapData)
{
	// ���o�ΏۂƂȂ�摜�̃r�b�g�}�b�v�f�[�^
	_source = bitmapData;

	// ���o���ꂽ�ʐ^�t���[���͈̔͂�ێ�����z������Z�b�g
	_frameRects.clear();

	// ���o���ꂽ�ʐ^�t���[���̃f�[�^��ێ�����z������Z�b�g
	for (auto region : _frameData)
	{
		delete region;
	}
	_frameData.clear();

	// ���o��������������`�悷�邽�߂̈ꎞ�I�ȃr�b�g�}�b�v�摜
	delete _tmp;
	_tmp = new Gdiplus::Bitmap(bitmapData->GetWidth(), bitmapData->GetHeight(), PixelFormat32bppARGB); // ���œh��Ԃ���Ă���Ɖ���
	{
		Gdiplus::Graphics g(_tmp);
		g.Clear(Gdiplus::Color(255, 255, 255));
	}

	for (UINT py = 0; py < _tmp->GetHeight() - _MIN_LENGTH; ++py)
	{
		for (UINT px = 0; px < _tmp->GetWidth() - _MIN_LENGTH; ++px)
		{
			if (isTransparence(px, py))
			{
				RECT rect;
				// (px, py)������
				// ���łɌ����ς݂̎ʐ^�t���[���͈͓̔��ɂ���ꍇ�͖�������
				BOOL flag = true;
				for ( RECT r : _frameRects)
				{
					POINT point = { (LONG)px, (LONG)py };
					if (PtInRect(&r, point)) //(/*rect.contains(px, py)*/)
					{
						px = r.right;
						flag = false;
						rect = r;
						break;
					}
				}

				if (flag)
				{
					// ���������̋��E���g���[�X
					rect = traceEdge(px, py);

					// �G���A�̉E�[����T�[�`���ĊJ����
					px = rect.right;
				}
			}
		}
	}
}

void PictureFrame::setFrame(std::vector<Gdiplus::Bitmap*>* listMask, double scale)
{
	// ���o���ꂽ�ʐ^�t���[���͈̔͂�ێ�����z������Z�b�g
	_frameRects.clear();

	// ���o���ꂽ�ʐ^�t���[���̃f�[�^��ێ�����z������Z�b�g
	for (auto region : _frameData)
	{
		delete region;
	}
	_frameData.clear();

	for (auto mask : *listMask)
	{
		Gdiplus::Bitmap* pTemp = (Gdiplus::Bitmap*)mask->GetThumbnailImage((UINT)(mask->GetWidth() * scale), (UINT)(mask->GetHeight() * scale));
		Gdiplus::Region* region = util::CreateRegionFromImage8bpp(pTemp);
		delete pTemp;
		_frameData.push_back(region);
	}
}

// ���������̋��E���g���[�X
RECT PictureFrame::traceEdge(UINT px, UINT py)
{
	POINT _startPoint = { (LONG)px, (LONG)py };

	// ���o�������������ɐԂ��s�N�Z����`�悵�Ă���
	_tmp->SetPixel(px, py, Gdiplus::Color(0, 0, 0)); //_tmp.setPixel32(px, py, 0xFFFF0000);

	// ���ׂ���W
	POINT _checkPoint = { (LONG)px, (LONG)py };

	// ���ׂ����(��0, �E1, ��2, ��3)
	UINT _checkDirection = 1;

	POINT minPoint = { (LONG)px, (LONG)py };
	POINT maxPoint = { (LONG)px, (LONG)py };
	for (;;)
	{
		BOOL loopBreake = TRUE;
		for (UINT d = 0; d < 4; ++d)
		{
			// ���ɒ��ׂ���W
			POINT p = pointOfDirection(_checkPoint, _checkDirection);

			if (p.x == _startPoint.x && p.y == _startPoint.y)
			{
				// �X�^�[�g�n�_�Ɠ����Ȃ�I��
				break;
			}
			else if (isTransparence(p.x, p.y))
			{
				// ���Wp�������Ȃ̂ŁAtmp�摜�ɐԂ��s�N�Z����`��
				_tmp->SetPixel(p.x, p.y, Gdiplus::Color(0, 0, 0)); //_tmp.setPixel32(px, py, 0xFFFF0000);

				// ���Wp��V���Ȋ�_�Ƃ��ăZ�b�g
				_checkPoint = p;
				_checkDirection = (_checkDirection + 3) % 4;
				if (p.x > maxPoint.x)
				{
					// �����͈͂̉E�[�̍��W���X�V�����Ȃ�A�X�V����
					maxPoint.x = p.x;
				}
				else if (p.x < minPoint.x)
				{
					// �����͈͂̍��[�̍��W���X�V�����Ȃ�A�X�V����
					minPoint.x = p.x;
				}
				if (p.y > maxPoint.y)
				{
					// �����͈͂̉��[�̍��W���X�V�����Ȃ�A�X�V����
					maxPoint.y = p.y;
				}
				else if (p.y < minPoint.y)
				{
					// �����͈͂̏�[�̍��W���X�V�����Ȃ�A�X�V����
					minPoint.y = p.y;
				}

				loopBreake = FALSE;
				break;
			}
			else
			{
				// ��������Ȃ��̂ŁA������ς���
				_checkDirection = (_checkDirection + 1) % 4;
			}
		}
		if (loopBreake)
		{
			break;
		}
	}

	// �g���[�X����
	static RECT rect;
	SetRect(&rect, minPoint.x, minPoint.y, minPoint.x + (maxPoint.x - minPoint.x) + 1, minPoint.y + (maxPoint.y - minPoint.y) + 1);
	if (rect.right - rect.left > _MIN_LENGTH && rect.bottom - rect.top > _MIN_LENGTH)
	{
		// ����ꂽ�͈͂�_MIN_LENGTH ���傫����Ύʐ^�t���[���Ƃ��ď�������

		// ���g�ň͂܂ꂽ�G���A�𔒂œh��Ԃ�
		FloodFill(_tmp, Gdiplus::Point(rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2));

		_frameRects.push_back(rect);

		// �ʐ^�t���[���̃T�C�Y�݂̂�؂�o�����摜�𐶐�
		Gdiplus::Bitmap* clip = new Gdiplus::Bitmap(_tmp->GetWidth(), _tmp->GetHeight(), PixelFormat8bppIndexed);
		{
			Gdiplus::Graphics g(clip);
			g.DrawImage(_tmp, 0, 0, 0, 0, _tmp->GetWidth(), _tmp->GetHeight(), Gdiplus::Unit::UnitPixel);
		}

		// PNG�摜��
		Gdiplus::Region *r = util::CreateRegionFromImage8bpp(clip);
		_frameData.push_back(r);
		delete clip;

		//tmp�摜���N���A����
		{
			Gdiplus::Graphics g(_tmp);
			g.Clear(Gdiplus::Color(255, 255, 255));
		}
	}
	return rect;
}

// ���Wp�������direction�ɂ�����W��Ԃ�
POINT PictureFrame::pointOfDirection(POINT p, UINT direction)
{
	static POINT d;
	d.x = p.x;
	d.y = p.y;
	switch (direction)
	{
	case 0:
		d.y = p.y - 1;
		break;
	case 1:
		d.x = p.x + 1;
		break;
	case 2:
		d.y = p.y + 1;
		break;
	case 3:
		d.x = p.x - 1;
		break;
	}
	return d;
}

BOOL PictureFrame::isTransparence(UINT px, UINT py)
{
	if (px >= 0 && px < _source->GetWidth() && py >= 0 && py < _source->GetHeight())
	{
		// �s�N�Z���̃A���t�@�l���擾
		Gdiplus::Color pixelValue;
		_source->GetPixel(px, py, &pixelValue);
		UINT alphaValue = pixelValue.GetAlpha();

		if (px <= 2 || px >= _source->GetWidth() - 2 || py <= 2 || py >= _source->GetHeight() - 2)
		{
			// �摜�̎��͂�������ƕs�����ɂ���Ă��Ȃ��P�[�X�����X����̂ŁA����2px�̓A���t�@0�̏ꍇ�̂݁u�����v�Ɣ���
			return alphaValue == 0;
		}
		else
		{
			// ���S�ȕs�����łȂ���΁A�u�����v�Ɣ���
			return alphaValue < 255;
		}
	}
	else
	{
		// �摜�̃G���A�O�́u��������Ȃ��v
		return FALSE;
	}
}

bool *PixelsChecked;

int CoordsToIndex(int x, int y, int stride)
{
	return (stride*y) + (x * 4);
}

BOOL CheckPixel(byte* px, byte* startcolor)
{
	return (px[0] == startcolor[0]);
}

void LinearFloodFill4(byte* scan0, int x, int y, Gdiplus::Size bmpsize, int stride, byte* startcolor)
{
	int* p = (int*)(scan0 + (CoordsToIndex(x, y, stride)));

	int LFillLoc = x;
	int* ptr = p;
	for (;;)
	{
		ptr[0] = Gdiplus::Color::Black;
		PixelsChecked[y*(bmpsize.Width + 1) + LFillLoc] = true;
		LFillLoc--;
		ptr -= 1;
		if (LFillLoc <= 0 ||
			!CheckPixel((byte*)ptr, startcolor) ||
			(PixelsChecked[y*(bmpsize.Width + 1) + LFillLoc]))
			break;
	}
	++LFillLoc;

	int RFillLoc = x;
	ptr = p;
	for (;;)
	{
		ptr[0] = Gdiplus::Color::Black;
		PixelsChecked[y*(bmpsize.Width + 1) + RFillLoc] = true;
		RFillLoc++;
		ptr += 1;
		if (RFillLoc >= bmpsize.Width ||
			!CheckPixel((byte*)ptr, startcolor) ||
			(PixelsChecked[y*(bmpsize.Width + 1) + RFillLoc]))
			break;
	}
	--RFillLoc;

	ptr = (int*)(scan0 + CoordsToIndex(LFillLoc, y, stride));
	for (int i = LFillLoc; i <= RFillLoc; ++i)
	{
		if (y>0 &&
			CheckPixel((byte*)(scan0 + CoordsToIndex(i, y - 1, stride)), startcolor) &&
			(!(PixelsChecked[(y - 1)*(bmpsize.Width + 1) + i])))
			LinearFloodFill4(scan0, i, y - 1, bmpsize, stride, startcolor);
		if (y<(bmpsize.Height - 1) &&
			CheckPixel((byte*)(scan0 + CoordsToIndex(i, y + 1, stride)), startcolor) &&
			(!(PixelsChecked[(y + 1)*(bmpsize.Width + 1) + i])))
			LinearFloodFill4(scan0, i, y + 1, bmpsize, stride, startcolor);
		ptr += 1;
	}
}

void FloodFill(Gdiplus::Bitmap* bmp, Gdiplus::Point pt)
{
	Gdiplus::BitmapData bmpData;
	bmp->LockBits(
		&Gdiplus::Rect(0, 0, bmp->GetWidth(), bmp->GetHeight()),
		Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite,
		PixelFormat32bppARGB, &bmpData);
	VOID* Scan0 = bmpData.Scan0;
	{
		byte * scan0 = (byte *)(void *)Scan0;
		int loc = CoordsToIndex(pt.X, pt.Y, bmpData.Stride);
		int color = *((int*)(scan0 + loc));
		PixelsChecked = new bool[(bmpData.Width + 1)*(bmpData.Height + 1)];
		ZeroMemory(PixelsChecked, sizeof(bool)*((bmpData.Width + 1)*(bmpData.Height + 1)));
		LinearFloodFill4(scan0, pt.X, pt.Y,
			Gdiplus::Size(bmpData.Width, bmpData.Height),
			bmpData.Stride,
			(byte*)&color);
		delete[]PixelsChecked;
	}
	bmp->UnlockBits(&bmpData);
}
