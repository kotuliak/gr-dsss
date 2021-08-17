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

    using input_type = byte;
    using output_type = byte;
    descramble_dsss_bb::sptr
    descramble_dsss_bb::make(int spread_symbol_length_, int spreading_code_length_)
    {
      return gnuradio::make_block_sptr<descramble_dsss_bb_impl>(spread_symbol_length_, spreading_code_length_);
    }


    /*
     * The private constructor
     */
    descramble_dsss_bb_impl::descramble_dsss_bb_impl(int spread_symbol_length_, int spreading_code_length_)
      : gr::block("descramble_dsss_bb",
              gr::io_signature::make(2 /* min inputs */, 2 /* max inputs */, sizeof(input_type)),
              gr::io_signature::make(1 /* min outputs */, 1 /*max outputs */, sizeof(output_type))),
              spread_symbol_length(spread_symbol_length_),
              spreading_code_length(spreading_code_length_),
              delay_distribution(spreading_code_length_, 0)
    {
      offset          = 0;
      delay           = 0;
      delay_zero_count = 0;
    }

    /*
     * Our virtual destructor.
     */
    descramble_dsss_bb_impl::~descramble_dsss_bb_impl()
    {
      for (auto i = 0; i < spreading_code_length; i++)
        cout << delay_distribution[i] << " ";
      cout << "\n";
      cout << spreading_code_length - distance(delay_distribution.begin(), max_element(delay_distribution.begin(), delay_distribution.end())) << "\n";
    }

    void
    descramble_dsss_bb_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      if (spread_symbol_length > spreading_code_length) {
        ninput_items_required[0] = noutput_items * spread_symbol_length;
        ninput_items_required[1] = noutput_items * spread_symbol_length;
      } else {
        ninput_items_required[0] = noutput_items * spreading_code_length;
        ninput_items_required[1] = noutput_items * spreading_code_length;
      }
    }

    int
    descramble_dsss_bb_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const input_type *in = reinterpret_cast<const input_type*>(input_items[0]);
      const input_type *in_2 = reinterpret_cast<const input_type*>(input_items[1]);
      output_type *out = reinterpret_cast<output_type*>(output_items[0]);

      int multiplier = (spreading_code_length < spread_symbol_length) ? spread_symbol_length / spreading_code_length : 1;

      vector<byte> input(in, in + spreading_code_length * multiplier);
      vector<byte> spreading_code(in_2, in_2 + spreading_code_length * multiplier);

      if (delay_zero_count <= 100) {

        vector<float> input_f;
        vector<float> spreading_code_f;

        float mean_spreading_code = (float) accumulate(spreading_code.begin(), spreading_code.end(), 0.0);
        mean_spreading_code = mean_spreading_code * 2 - spreading_code.size(); 
        mean_spreading_code /= (float) spreading_code.size();

        float mean_input = (float) accumulate(input.begin(), input.end(), 0.0);
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

      int no_output = 0;
      
      if (delay != 0) {
        consume(1, delay);
      } else {

        int mean_val = 0;
        for (auto i = 0; i < spreading_code_length * multiplier; i++) {
          mean_val += (int) (input[i] ^ spreading_code[i]);
          if ((i + 1) % spread_symbol_length == 0) {
            out[(i + 1) / spread_symbol_length - 1] = (byte) round((float) mean_val / (float) spread_symbol_length);
            no_output++;
          }
          
        }
        consume_each(spreading_code_length * multiplier);
      }

      // Tell runtime system how many output items we produced.
      return no_output;
    }

  } /* namespace dsss */
} /* namespace gr */

