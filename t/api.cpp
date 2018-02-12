#include "gtest/gtest.h"
/*
#include <memory>
#include <list>
#include <utility>
#include <iostream>
#include <string>
#include <exception>
#include <functional>
*/
#include "reprocpp/after.h"
#include "test.h"
//#include "promise/asio/loop.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include <reprocurl/api.h>
#include <signal.h>

#include <openssl/pem.h>
/*
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bn.h>
*/
#ifdef _WIN32
#include <openssl/applink.c>
#endif


using namespace prio;
using namespace reprocurl;


class APITest : public ::testing::Test {
 protected:

  virtual void SetUp() 
  {}

  virtual void TearDown() 
  {}
}; // end test setup


TEST_F(APITest, SimpleHttp) {

	int status = 0;
	std::string header;
	{
        auto req = request(prio::Url("https://www.amazon.de/"));

        fetch(req)
        .then([&status,&header](response res)
        {
			status = res.status();
			header = res.header("server");

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
		curl_multi().dispose();
	}


	EXPECT_EQ(200,status);
	EXPECT_EQ("Server",header);
    MOL_TEST_ASSERT_CNTS(0, 0);
}

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED

Future<> coroutine_example()
{
	try 
    {
        auto req = request(prio::Url("https://www.google.de/"));

        response res = co_await fetch(req);

		int status = res.status();
		std::string header = res.header("server");

    	EXPECT_EQ(200,status);
	    EXPECT_EQ("Server",header);

		theLoop().exit();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		theLoop().exit();
	};
}


TEST_F(APITest, asyncTest)
{
	{
		coroutine_example();

		theLoop().run();
		curl_multi().dispose();
	}

	MOL_TEST_ASSERT_CNTS(0, 0);
}


#endif


int main(int argc, char **argv) 
{
	prio::init();
    reprocurl::init();

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}
