#include "gtest/gtest.h"
#include "reprocpp/after.h"
#include "test.h"
#include "priocpp/api.h"
#include "priocpp/task.h"
#include <reprocurl/api.h>
#include <signal.h>
#include <openssl/pem.h>

#ifdef _WIN32
#include <openssl/applink.c>
#endif


using namespace repro;
using namespace prio;
using namespace reprocurl;


class APITest : public ::testing::Test {
 protected:

  virtual void SetUp() 
  {}

  virtual void TearDown() 
  {}
}; // end test setup


TEST_F(APITest, SimpleHttp) 
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

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

        // can only call that once
		//curl_multi().dispose();
	}

	EXPECT_EQ(200,status);
	EXPECT_EQ("Server",header);
    // we did not call curl_multi().dispose(), so this won't assert
    //MOL_TEST_ASSERT_CNTS(0, 0);
}

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED

Future<> coroutine_example()
{
	try 
    {
        request req(prio::Url("https://www.google.de/"));

        response res = co_await fetch(req);
std::cout << "1<<<<<<<<<<<<<" << std::endl;
    	int status = res.status();
		std::string header = res.header("server");
std::cout << "2<<<<<<<<<<<<<" << std::endl;

    
    	//EXPECT_EQ(200,status);
	   // EXPECT_EQ("gws",header);
    
		nextTick([](){
			theLoop().exit();
		});
    }
	catch (const std::exception& ex)
	{
        std::cout << "0<<<<<<<<<<<<<" << std::endl;
		std::cout << ex.what() << std::endl;
		theLoop().exit();
	}
    catch (...)
	{
        std::cout << "oioioi" << std::endl;
		theLoop().exit();
	};
    //co_return 42;
}


TEST_F(APITest, asyncTest)
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

	{
		coroutine_example().then([](){});

		theLoop().run();
		curl_multi().dispose();
	}

	MOL_TEST_ASSERT_CNTS(0, 0);
}


#endif


TEST_F(APITest, SimpleHttp2) 
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

	int status = 200;
	{
        auto req1 = request(prio::Url("https://www.amazon.de/"));
        auto req2 = request(prio::Url("https://www.amazon.de/"));

		std::vector<request> requests {req1,req2};

		fetch_all(requests)
		.then([&status](std::vector<response> responses)
		{
			for ( auto& r : responses)
			{
				if(status==200) status = r.status();
			}

			theLoop().exit();
		})
		.otherwise([](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});

		theLoop().run();

        // can only call that once
		//curl_multi().dispose();
	}

	EXPECT_EQ(200,status);
    // we did not call curl_multi().dispose(), so this won't assert
    //MOL_TEST_ASSERT_CNTS(0, 0);
}

TEST_F(APITest, SimpleHttp3) 
{

#ifndef _WIN32
	signal(SIGPIPE).then([](int s){ std::cout << "SIGPIPE" << std::endl;});
#endif
	signal(SIGINT).then([](int s) { theLoop().exit(); });

	int status = 200;
	{
        auto req1 = request(prio::Url("https://www.amazon.de/"));
        auto req2 = request(prio::Url("https://www.amazon.de/"));

		after(
			fetch(req1),
			fetch(req2)
		)
		.then([&status](std::tuple<response,response> responses)
		{
			response res1;
			response res2;
			std::tie(res1,res2) = responses;

			if(status==200) status = res1.status();
			if(status==200) status = res2.status();

			theLoop().exit();
		})
		.otherwise([](const std::exception& ex)
		{
			std::cout << ex.what() << std::endl;
		});

		theLoop().run();

        // can only call that once
		//curl_multi().dispose();
	}

	EXPECT_EQ(200,status);
    // we did not call curl_multi().dispose(), so this won't assert
    //MOL_TEST_ASSERT_CNTS(0, 0);
}

int main(int argc, char **argv) 
{

	prio::init();
	OpenSSL_add_all_algorithms();
    reprocurl::init();


    ::testing::InitGoogleTest(&argc, argv);
    int r = RUN_ALL_TESTS();

    return r;
}
