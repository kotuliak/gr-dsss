/* -*- c++ -*- */
/*
 * Copyright 2021 gr-dsss author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gnuradio/io_signature.h>
#include "descramble_dsss_bb_impl.h"
#include <cmath>
#include <numeric>

using namespace std;

namespace gr {
  namespace dsss {

    using input_type = uint8_t;
    using output_type = uint8_t;
    descramble_dsss_bb::sptr
    descramble_dsss_bb::make(int spread_symbol_length_, gr_vector_int &spreading_code_)
    {
      return gnuradio::make_block_sptr<descramble_dsss_bb_impl>(spread_symbol_length_, spreading_code_);
    }

    boost::dynamic_bitset<> concatenate(const boost::dynamic_bitset<>& first, const boost::dynamic_bitset<>& second)
    {
      boost::dynamic_bitset<> value(first);
      //Increase the size of the bit buffer to fit the data being placed in it
      value.resize(first.size() + second.size());
      value <<= second.size();
      for (size_t i = 0; i < second.size(); i++)
      {
        value[i] = second[i];
      }
      return value;
    }


    /*
     * The private constructor
     */
    descramble_dsss_bb_impl::descramble_dsss_bb_impl(int spread_symbol_length_, gr_vector_int &spreading_code_)
      : gr::block("descramble_dsss_bb",
              gr::io_signature::make(1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
              gr::io_signature::make(1 /* min outputs */, 1 /*max outputs */, sizeof(output_type))),
              spread_symbol_length(spread_symbol_length_)
    {
      offset          = 0;
      delay           = 0;
      delay_zero_count = 0;
      spreading_code_length = spreading_code_.size() * 8;
      delay_distribution = vector<float>(spreading_code_length, 0);

      for (size_t i = 0; i < spreading_code_.size(); i++)
      {
        boost::dynamic_bitset<> byte_ar(8, spreading_code_[i]);
        
        spreading_code = concatenate(spreading_code, byte_ar);
      }
    }

    /*
     * Our virtual destructor.
     */
    descramble_dsss_bb_impl::~descramble_dsss_bb_impl()
    {
      for (auto i = 0; i < spreading_code_length; i++)
        cout << delay_distribution[i] << "\n";
      cout << "\n";
      cout << spreading_code_length - distance(delay_distribution.begin(), max_element(delay_distribution.begin(), delay_distribution.end())) << "\n";
      cout << spreading_code << "\n";
    }

    void
    descramble_dsss_bb_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items * spreading_code_length / 2;
    }

    int
    descramble_dsss_bb_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const input_type *in = reinterpret_cast<const input_type*>(input_items[0]);
      output_type *out = reinterpret_cast<output_type*>(output_items[0]);

      // int multiplier = (spreading_code_length < spread_symbol_length) ? spread_symbol_length / spreading_code_length : 1;
      int multiplier = 1;
      int no_output = 0;

      for (size_t j = 0; j < noutput_items; j++)
      {
      

        boost::dynamic_bitset<> input;

        for (size_t i = 0; i < spreading_code_length / 2; i++)
        {
          boost::dynamic_bitset<> byte_arr(2, in[j * (spreading_code_length / 2) + i]);
          input = concatenate(input, byte_arr);
        }

        

        if (delay_zero_count <= 100) {

          vector<float> input_f;
          vector<float> spreading_code_f;

          float mean_spreading_code = spreading_code.count();
          mean_spreading_code = mean_spreading_code * 2 - spreading_code.size(); 
          mean_spreading_code /= (float) spreading_code.size();

          float mean_input = input.count();
          mean_input = mean_input * 2 - input.size();
          mean_input /= (float) input.size();

          for (int i = 0; i < spreading_code.size(); i++) {
            input_f.push_back(2 * ((float) input[i] - 0.5) - mean_input);
            
            spreading_code_f.push_back( 2 * ((float) spreading_code[i] - 0.5) - mean_spreading_code);
          }

          // compute cross correlation so we find best delay
          for (int i = 0; i < spreading_code_length; i++) {
            float product_can = abs(inner_product(input_f.begin(), input_f.end(), spreading_code_f.begin(), 0.0));
            delay_distribution[i] += product_can; 

            rotate(spreading_code_f.begin(), spreading_code_f.begin() + 1, spreading_code_f.end());
            
          }
          
          delay = distance(delay_distribution.begin(), max_element(delay_distribution.begin(), delay_distribution.end()));
          rotate(delay_distribution.begin(), delay_distribution.begin() + delay, delay_distribution.end());

          if (delay) {
            delay_zero_count = 0;
          } else {
            delay_zero_count++;
          }

        }

        // std::cout<<delay_zero_count<<"\n";

        
        
        if (delay != 0) {
          consume_each(delay / 2);
          return no_output;
        } else {
          float mean_val = ((float) ((input ^ spreading_code).count())) / ((float) spreading_code_length);
          out[no_output] = (uint8_t) round(mean_val);
          no_output++;
          consume_each(spreading_code_length / 2);
        }

        // cout << (input ^ spreading_code) << " " << noutput_items << " " << delay << "\n";

      }

      // // Tell runtime system how many output items we produced.
      // std::cout << multiplier << std::endl;
      return no_output;
    }

  } /* namespace dsss */
} /* namespace gr */

