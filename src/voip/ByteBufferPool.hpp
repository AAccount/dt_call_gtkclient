/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ByteBufferPool.hpp
 * Author: Daniel
 *
 * Created on January 20, 2020, 8:34 PM
 */

#ifndef BYTEBUFFERPOOL_HPP
#define BYTEBUFFERPOOL_HPP

#include <memory>
#include <vector>
#include <mutex>

#include "../vars.hpp"

class ByteBufferPool
{
public:
	explicit ByteBufferPool(int bufferSize);
	ByteBufferPool(const ByteBufferPool& orig) = delete; //you can't copy someone else's vector of unique pointers
	virtual ~ByteBufferPool();
	
	std::unique_ptr<unsigned char[]> getBuffer();
	void returnBuffer(std::unique_ptr<unsigned char[]>& buffer);
	int getBufferSize() const;
private:
	std::vector<std::unique_ptr<unsigned char[]>> buffers;
	std::mutex btex;
	int size = 10;
	int bufferSize;
	void generateBuffers();
};

#endif /* BYTEBUFFERPOOL_HPP */

