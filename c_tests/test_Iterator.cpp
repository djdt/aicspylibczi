//
// Created by Jamie Sherman on 2019-09-10.
//

#include <vector>

#include "catch.hpp"
#include "Iterator.h"
#include "helper_algorithms.h"

using namespace pylibczi;

TEST_CASE("iterator_source", "[iterator_src]"){
    uint16_t src[60];  // w=5 h=4 c=3 (rgb, bgr or whatever)
    for( int i = 0; i < 60; i++){
        src[i] = i%3 + 1; // map the values to 1,2,3,1,2,3 ... so that the channel structure is embedded by value.
    }
    SourceRange<uint16_t> sourceRange(3, src, src + 60, 2*5*3, 5);

    for( int i = 0; i < 4; i++ ){
        auto begin = sourceRange.stride_begin(i);
        REQUIRE( *((*begin)[0]) == 1 );
        REQUIRE( *((*begin)[1]) == 2 );
        REQUIRE( *((*begin)[2]) == 3 );
    }
}

TEST_CASE("iterator_source_to_target", "[iterator_src_tgt]"){
    uint16_t src[60], tgt[60];  // w=5 h=4 c=3 (rgb, bgr or whatever)
    for( int i = 0; i < 60; i++){
        src[i] = i%3 + 1; // map the values to 1,2,3,1,2,3 ... so that the channel structure is embedded by value.
        tgt[i] = 0;
    }
    SourceRange<uint16_t> sourceRange(3, src, src + 60, 2*3*5, 5);
    TargetRange<uint16_t> targetRange( 3, 5, 4, tgt, tgt+60 );

    for(int i = 0; i < 4; i++){ // images are apparently copied by stride due to possible padding so we follow that paradigm
        paired_for_each(sourceRange.stride_begin(i), sourceRange.stride_end(i), targetRange.stride_begin(i),
            [&tgt](std::vector<uint16_t *>a, std::vector<uint16_t *>b){
          paired_for_each(a.begin(), a.end(), b.begin(), [&tgt](uint16_t *s, uint16_t *t){
              int val = tgt[0];
              *t = *s;
          });
        });
    }
    for(int i = 0; i < 60; i++)
        REQUIRE( tgt[i] == i/20 +1 );
}

