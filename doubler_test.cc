#include <gnuradio/block.h>
#include <gnuradio/blocks/vector_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/logger.h>
#include <gnuradio/top_block.h>
#include <pmt/pmt.h>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test suite
#include <boost/test/unit_test.hpp>

#include <algorithm>

using ios = gr::io_signature;

class doubler : virtual public gr::block
{
public:
    doubler()
        : gr::block("my_doubler",
                    ios::make(1, 1, sizeof(float)),
                    ios::make(1, 1, sizeof(float))){};
    ~doubler(){};
    int general_work(int noutput,
                     gr_vector_int& ninput,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items)
    {
        static pmt::pmt_t symbol = pmt::mp("test");
        auto in_ptr = reinterpret_cast<const float*>(input_items[0]);
        auto out_ptr = reinterpret_cast<float*>(output_items[0]);
        auto items_to_process = std::min(ninput[0], noutput);
        d_logger->debug("general_work(): Items to process: {}", items_to_process);

        for (auto in = in_ptr; in != in_ptr + items_to_process; ++in) {
            *(out_ptr++) = 2 * *in;
        }

        consume_each(items_to_process);
        add_item_tag(0, 0, symbol, pmt::PMT_T);
        return items_to_process;
    }
};

BOOST_AUTO_TEST_CASE(lambda_block_run)
{
    auto tb = gr::make_top_block("top");
    gr::logger logger("lambda_block_run");

    std::vector<float> input_data{ 0.0f, 1.0f, 2.0f, 4.0f };
    std::vector<float> expected;
    expected.reserve(input_data.size());
    for (const auto in : input_data) {
        expected.push_back(in * 2);
    }

    logger.debug("Setting up blocks...");
    auto src = gr::blocks::vector_source_f::make(input_data);
    auto dst = gr::blocks::vector_sink_f::make();
    auto dut = gnuradio::make_block_sptr<doubler>();

    logger.debug("Connecting blocks...");
    tb->connect(src, 0, dut, 0);
    tb->connect(dut, 0, dst, 0);

    logger.debug("Running flow graph...");
    tb->run();

    BOOST_TEST(dst->data() == expected, boost::test_tools::per_element());
    auto tags = dst->tags();
    BOOST_CHECK_EQUAL(tags.size(), 1);
    BOOST_CHECK_EQUAL(tags[0].offset, 0);
    BOOST_CHECK_EQUAL(tags[0].key, pmt::string_to_symbol("test"));
}
