/* -*- c++ -*- */
/*
 * Copyright 2021 gr-dsss author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_DSSS_DESCRAMBLE_DSSS_BB_H
#define INCLUDED_DSSS_DESCRAMBLE_DSSS_BB_H

#include <dsss/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace dsss {

    /*!
     * \brief <+description of block+>
     * \ingroup dsss
     *
     */
    class DSSS_API descramble_dsss_bb : virtual public gr::block
    {
     public:
      typedef std::shared_ptr<descramble_dsss_bb> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of dsss::descramble_dsss_bb.
       *
       * To avoid accidental use of raw pointers, dsss::descramble_dsss_bb's
       * constructor is in a private implementation
       * class. dsss::descramble_dsss_bb::make is the public interface for
       * creating new instances.
       */
      static sptr make(int spread_symbol_length_, int spreading_code_length_);
    };

  } // namespace dsss
} // namespace gr

#endif /* INCLUDED_DSSS_DESCRAMBLE_DSSS_BB_H */

