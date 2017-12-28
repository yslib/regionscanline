#include "framebuffer.h"

void FrameBuffer::setColor24(const Point2d<int>& p, const Color24 & color24)
{
	setColor24(std::get<0>(p), std::get<1>(p), color24);
}

void FrameBuffer::setColor32(const Point2d<int>& p, const Color32 & color32)
{
	setColor32(std::get<0>(p), std::get<1>(p), color32);
}

void FrameBuffer::setColor24(const Color24 &color24)
{
	for (int i = 0; i < m_height; i++) {
		for (int j = 0; j < m_width; j++) {
			int index = j + i * m_width;
			m_buffer[3*index] = std::get<0>(color24);
			m_buffer[3*index + 1] = std::get<1>(color24);
			m_buffer[3*index + 2] = std::get<2>(color24);
		}
	}
}

void FrameBuffer::setColor32(const Color32 &color32)
{
	for (int i = 0; i < m_height; i++) {
		for (int j = 0; j < m_width; j++) {
			int index = j + i * m_width;
			m_buffer[4*index] = std::get<0>(color32);
			m_buffer[4*index + 1] = std::get<1>(color32);
			m_buffer[4*index + 2] = std::get<2>(color32);
			m_buffer[4*index + 3] = std::get<3>(color32);
		}
	}
}
void FrameBuffer::setColor24(int x, int y, const Color24 &color24)
{
	setColor24(x + y * m_width, color24);
}

void FrameBuffer::setColor32(int x, int y, const Color32 &color32)
{
	setColor32(x + y * m_width, color32);

}

void FrameBuffer::setColor24(int index, const Color24 &color24)
{
	m_buffer[3*index] = std::get<0>(color24);
	m_buffer[3*index + 1] = std::get<1>(color24);
	m_buffer[3*index + 2] = std::get<2>(color24);
}

void FrameBuffer::setColor32(int index, const Color32 &color32)
{
	m_buffer[4*index] = std::get<0>(color32);
	m_buffer[4*index + 1] = std::get<1>(color32);
	m_buffer[4*index + 2] = std::get<2>(color32);
	m_buffer[4*index + 3] = std::get<3>(color32);
}

void FrameBuffer::setHorizontialLineColor24(int y, int startx, int endx, const Color24 & color24)
{
	for (int x = startx; x < endx; x++) {
		int index = y * m_width + x;
		m_buffer[3 * index] = std::get<0>(color24);
		m_buffer[3 * index + 1] = std::get<1>(color24);
		m_buffer[3 * index + 2] = std::get<2>(color24);
	}
}
