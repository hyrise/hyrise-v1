#ifndef GTEST_BENCHMARK_H
#define GTEST_BENCHMARK_H

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>


// Helper macro for defining tests.
#define GTEST_BENCHMARK_(test_case_name, test_name, parent_class, parent_id) \
    class GTEST_TEST_CLASS_NAME_(test_case_name, test_name) : public parent_class { \
    public:                                                             \
        GTEST_TEST_CLASS_NAME_(test_case_name, test_name)() {}          \
    private:                                                            \
        virtual void BenchmarkBody();                                   \
        static ::testing::TestInfo* const test_info_ GTEST_ATTRIBUTE_UNUSED_; \
        GTEST_DISALLOW_COPY_AND_ASSIGN_(                                \
            GTEST_TEST_CLASS_NAME_(test_case_name, test_name));         \
    };                                                                  \
                                                                        \
                                                                        \
    ::testing::TestInfo* const GTEST_TEST_CLASS_NAME_(test_case_name, test_name) \
        ::test_info_ =                                                  \
        ::testing::internal::MakeAndRegisterTestInfo(                   \
            #test_case_name, #test_name, NULL, NULL,                    \
            (parent_id),                                                \
            parent_class::SetUpTestCase,                                \
            parent_class::TearDownTestCase,                             \
            new ::testing::internal::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(test_case_name, test_name)>); \
                                                                        \
    void GTEST_TEST_CLASS_NAME_(test_case_name, test_name)::BenchmarkBody()


namespace testing
{
    class BenchmarkPrinter : public ::testing::EmptyTestEventListener 
    {
        // Called before a test starts.
        virtual void OnTestStart(const ::testing::TestInfo& test_info) 
        {
            printf("[ RUN      ] ");
            printf(" %s.%s \n", test_info.test_case_name(), test_info.name());
        }

        // Called after a test ends.
        virtual void OnTestEnd(const ::testing::TestInfo& test_info) 
        {
            const TestResult* tr = test_info.result();
            for(int i=0; i < tr->test_property_count(); ++i)
            {
                printf("[      MSG ] %s : %s\n", tr->GetTestProperty(i).key(), tr->GetTestProperty(i).value());
            }
            printf("[       OK ]\n");
        }

    };

    class BenchmarkPublisher : public ::testing::EmptyTestEventListener 
    {
        ::std::string const filename;
        ::std::stringstream output;
    public:
        BenchmarkPublisher(std::string _filename) : filename(_filename)
        {
        }

        // Called after everything is done
        virtual void OnTestProgramEnd(const UnitTest&)
        {
            std::ofstream out(filename);
            out << output.str();
        }

        // Called after all perf regressions end
        virtual void OnTestIterationEnd(const UnitTest& unit_test, int iteration)
	    {
	        ::std::stringstream jsonString;

	        jsonString << "[";

	        for(int i = 0; i < unit_test.total_test_case_count(); ++i)
	        {
		        const TestCase& test_case = *unit_test.GetTestCase(i);
		        for(int j = 0; j < test_case.total_test_count(); ++j)
		        {
		            jsonString << "{";
		            jsonString << "\"commitid\" : \"__GITID__\",";
		            jsonString << "\"project\" : \"HYRISE\",";
		            jsonString << "\"branch\" : \"__BRANCH__\",";
		            jsonString << "\"environment\" : \"__ENV__\",";
		 
		            jsonString << attachResults(*test_case.GetTestInfo(j));
		   
		            if(j != test_case.total_test_count() - 1)
		            {
		                jsonString << "},";
		            }
		            else
		            {
		                jsonString << "}";
		            }
		        }
                if(i != unit_test.total_test_case_count() - 1)
                {
                    jsonString << ",";
                }
	        }   
	        jsonString << "]\n";

	        output << jsonString.str();
	    }

	    ::std::string attachResults(const TestInfo& test_info)
	    {
	        ::std::stringstream ss;
	        const TestResult* tr = test_info.result();
           
            ss << "\"benchmark\" : \"" << test_info.test_case_name() << "." << test_info.name() << "\",";
	        ss << "\"executable\" : \"" << "__EXECUTABLE__" << "\",";

	        for(int i = 0; i < tr->test_property_count(); ++i)
	        {
	            //should be patched with a GetTestPropertyByName method
	            if(strcmp(tr->GetTestProperty(i).key(), "AVG") == 0)
	    	    {
		            ss << "\"result_value\" : " << tr->GetTestProperty(i).value();		    
		        }

	    	    else if(strcmp(tr->GetTestProperty(i).key(), "STDDEV") == 0)
		        {
		            ss << "\"std_dev\" : " << tr->GetTestProperty(i).value();
		        }

		        else if(strcmp(tr->GetTestProperty(i).key(), "MIN") == 0)
		        {
		            ss << "\"min\" : " << tr->GetTestProperty(i).value();
		        }

		        else if(strcmp(tr->GetTestProperty(i).key(), "MAX") == 0)
		        {
		            ss << "\"max\" : " << tr->GetTestProperty(i).value();
		        }

		        if(i != tr->test_property_count() - 1)
		        {
		            ss << ",";
		        }
	        }

	        return ss.str();
	    }
    };

    class Benchmark : public ::testing::Test
    {

        // How often a benchmark is executed
        int _numIterations;
        // Number of Warmup Iteratiosn
        int _warmUp;
        // Which additional PAPI Event to trace
        std::string _papi_event;

    public:

        Benchmark(): Test(), _numIterations(10), _warmUp(1), _papi_event("PAPI_TOT_INS")
        {}

        // Helper methods to set the iterations and warmup
        void SetNumIterations(int n) { _numIterations = n; }
        int NumIterations() { return _numIterations; }

        void SetWarmUp(int n) { _warmUp = n; }
        int WarmUp() { return _warmUp; }
        void logValue(const std::string& key, double value) const;
        void SetPapiEvent(std::string p) { _papi_event = p; }
        std::string PapiEvent() { return _papi_event; }
        
        // Needs to be implemented by the subclass
        virtual void BenchmarkBody() = 0;

        // This is the base method that is called from the original
        // test fixture
        virtual void TestBody();

        virtual void BenchmarkSetUp(){}

        virtual void BenchmarkTearDown(){}

    };

    // Define a new benchmark identified by the number of repetitions to be executed 
#define BENCHMARK( test_name )                  \
    GTEST_BENCHMARK_(Benchmark, test_name, testing::Benchmark,          \
                     ::testing::internal::GetTypeId<testing::Benchmark>())

#define BENCHMARK_F(test_fixture, test_name)                            \
    GTEST_BENCHMARK_(test_fixture, test_name, test_fixture,             \
                     ::testing::internal::GetTypeId<test_fixture>())




}

#endif // GTEST_BENCHMARK_H
