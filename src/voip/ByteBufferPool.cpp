/* 
 * File:   ByteBufferPool.cpp
 * Author: Daniel
 * 
 * Created on January 20, 2020, 8:34 PM
 */

#include "ByteBufferPool.hpp"

ByteBufferPool::ByteBufferPool(int bufferSize):
bufferSize(bufferSize)
{
	generateBuffers();
}

ByteBufferPool::~ByteBufferPool()
{
}

std::unique_ptr<unsigned char[]> ByteBufferPool::getBuffer()
{
	if(buffers.size() == 0)
	{
		generateBuffers();
	}
	std::unique_ptr<unsigned char[]> buffer = std::move(buffers[0]);
	buffers.erase(buffers.begin());
	return std::move(buffer);
}

void ByteBufferPool::returnBuffer(std::unique_ptr<unsigned char[]> buffer)
{
	buffers.push_back(std::move(buffer));
}

void ByteBufferPool::generateBuffers()
{
	for(int i=0; i<size; i++)
	{
		std::unique_ptr<unsigned char[]> buffer = std::make_unique<unsigned char[]>(bufferSize);
		buffers.push_back(std::move(buffer));
	}
	size = size*2;
}

int ByteBufferPool::getBufferSize() const
{
	return bufferSize;
}

