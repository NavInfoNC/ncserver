#include "stdafx.h"
#include "gtest.h"
#include "ncserver.h"
#include "mutable_service_io.h"

using namespace ncserver;

TEST(MutableServiceIo, read)
{
	MutableServiceIo io;
	const char* postData = "{\"io\":\"MutableServiceIo\"}";
	io.setPostData((void*)postData, strlen(postData));
	char* buffer = (char*)malloc(strlen(postData));
	io.read(buffer, strlen(postData));
	EXPECT_EQ(strncmp(buffer, postData, io.bufferSize()), 0);
}

TEST(MutableServiceIo, write)
{
	MutableServiceIo io;
	const char* result = "{\"io\":\"MutableServiceIo\"}";
	io.write((void*)result, strlen(result));
	EXPECT_EQ(strncmp((char*)io.buffer(), result, io.bufferSize()), 0);
}

TEST(MutableServiceIo, print)
{
	MutableServiceIo io;
	const char* result = "{\"io\":\"MutableServiceIo\"}";
	io.print("{\"%s\":\"%s\"}", "io", "MutableServiceIo");
	EXPECT_EQ(strncmp((char*)io.buffer(), result, io.bufferSize()), 0);
}

TEST(MutableServiceIo, headerField)
{
	MutableServiceIo io;
	const char* result = "Content-type: application/json\r\nContent-length: 334\r\n\r\n{\"io\":\"MutableServiceIo\"}";
	const char* buffer = "{\"io\":\"MutableServiceIo\"}";
	io.addHeaderField("Content-type: application/json");
	io.addHeaderField("Content-length: 334");
	io.endHeaderField();
	io.write((void*)buffer, strlen(buffer));
	EXPECT_EQ(strncmp((char*)io.buffer(), result, io.bufferSize()), 0);
}

TEST(MutableServiceIo, resetBuffer)
{
	MutableServiceIo io;
	const char* result = "Content-type: application/json\r\nContent-length: 334\r\n\r\n{\"io\":\"MutableServiceIo\"}";
	const char* buffer = "{\"io\":\"MutableServiceIo\"}";
	io.addHeaderField("Content-type: application/json");
	io.addHeaderField("Content-length: 334");
	io.endHeaderField();
	io.write((void*)buffer, strlen(buffer));
	EXPECT_EQ(strncmp((char*)io.buffer(), result, io.bufferSize()), 0);
	io.resetBuffer();
	io.addHeaderField("Content-type: application/json");
	io.addHeaderField("Content-length: 334");
	io.endHeaderField();
	io.write((void*)buffer, strlen(buffer));
	EXPECT_EQ(strncmp((char*)io.buffer(), result, io.bufferSize()), 0);
}