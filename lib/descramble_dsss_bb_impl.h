/* -*- c++ -*- */
/*
 * Copyright 2021 gr-dsss author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_DSSS_DESCRAMBLE_DSSS_BB_IMPL_H
#define INCLUDED_DSSS_DESCRAMBLE_DSSS_BB_IMPL_H

#include <boost/dynamic_bitset.hpp>
#include <dsss/descramble_dsss_bb.h>

#define byte uint8_t

using namespace std;

namespace gr {
  namespace dsss {

    class descramble_dsss_bb_impl : public descramble_dsss_bb
    {
     private:
      int delay;
      int offset;
      int spread_symbol_length;
      int spreading_code_length;
      int delay_zero_count;
      vector<float> delay_distribution;
      boost::dynamic_bitset<> spreading_code;

     public:
      descramble_dsss_bb_impl(int spread_symbol_length_, gr_vector_int &spreading_code_);
      ~descramble_dsss_bb_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

    };

  } // namespace dsss
} // namespace gr

#endif /* INCLUDED_DSSS_DESCRAMBLE_DSSS_BB_IMPL_H */

