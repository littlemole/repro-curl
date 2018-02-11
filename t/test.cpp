#include "gtest/gtest.h"
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
#include "reprocpp/after.h"
#include "test.h"
//#include "promise/asio/loop.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include <reprocurl/asyncCurl.h>
#include <curl/curl.h>
#include <signal.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#ifdef _WIN32
#include <openssl/applink.c>
#endif


using namespace prio;
using namespace reprocurl;


class BasicTest : public ::testing::Test {
 protected:


  virtual void SetUp() {
  }

  virtual void TearDown() {
//	    MOL_TEST_PRINT_CNTS();
	//	MOL_TEST_ASSERT_CNTS(0,0);
  }
}; // end test setup


TEST_F(BasicTest, SimpleHttp) {

	int status = 0;
	std::string header;
	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
		signal(SIGINT).then([](int s) { theLoop().exit(); });


		async_curl()
		->url("https://www.amazon.de/")
		->verbose()
		->perform()
		.then( [&status,&header](CurlEasy::Ptr curl)
		{
			status = curl->status();
			header = curl->response_header("server");
			// std::cout << curl->response_body() << std::endl;
			return async_curl()
			->url("https://www.amazon.de/")
			->perform();
		})
		.then( [&status,&header](CurlEasy::Ptr curl)
		{
			status = curl->status();
			header = curl->response_header("server");
			// std::cout << curl->response_body() << std::endl;

			return  timeout(1,0);
		})
		.then( []()
		{
			theLoop().exit();
		})
		.otherwise([](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});

		theLoop().run();
		//curl_multi().dispose();
	}


	//EXPECT_EQ(200,status);
	//EXPECT_EQ("gws",header);
	//MOL_TEST_ASSERT_CNTS(0,0);
}

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED

/*

*/

Future<> coroutine_example();

TEST_F(BasicTest, asyncTest)
{
	{
#ifndef _WIN32
		signal(SIGPIPE).then([](int s) { std::cout << "SIGPIPE" << std::endl;});
#endif
		signal(SIGINT).then([](int s) { theLoop().exit(); });

		coroutine_example();

		theLoop().run();
		curl_multi().dispose();
	}


	//EXPECT_EQ(200,status);
	//EXPECT_EQ("gws",header);
	MOL_TEST_ASSERT_CNTS(0, 0);
}


#endif


int main(int argc, char **argv) 
{
	prio::init();
	OpenSSL_add_all_algorithms();
	curl_global_init(CURL_GLOBAL_ALL);
    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED

Future<> coroutine_example()
{
	int status = 0;
	std::string header;

	try {

		auto req = async_curl()->url("https://www.google.de/");

		CurlEasy::Ptr curl = co_await req->perform();

		status = curl->status();
		header = curl->response_header("server");

		theLoop().exit();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		theLoop().exit();
	};

}

#endif
