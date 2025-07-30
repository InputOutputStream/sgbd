
#ifndef VECTORIZED_OPARATIONS_H

#include <immintrin.h>
#include <vector>
#include <cstdint>


class VectorizedOperations {

    public:

        // Compare 8 integers simultaneously using AVX2
        static std::vector <bool > compare_greater_than_vectorized(
        const std::vector <int32_t >& values , int32_t threshold) {

            std::vector <bool > results(values.size());

            const __m256i threshold_vec = _mm256_set1_epi32(threshold);
            size_t i = 0;

            // Process 8 integers at a time

            for (; i + 8 <= values.size(); i += 8) {

                __m256i values_vec = _mm256_loadu_si256(

                reinterpret_cast <const __m256i *>(& values[i]));

                __m256i cmp_result = _mm256_cmpgt_epi32(values_vec ,
                threshold_vec);
                // Extract comparison results

                int mask = _mm256_movemask_ps(_mm256_castsi256_ps(
                cmp_result));

                for (int j = 0; j < 8; ++j) {

                    results[i + j] = (mask & (1 << j)) != 0;

                }

            }

            // Handle remaining elements
            for (; i < values.size(); ++i) {

                results[i] = values[i] > threshold;

            }

            return results;

        }

    // Vectorized sum for double precision values

    static double sum_vectorized(const std::vector <double >& values) {
        __m256d sum_vec = _mm256_setzero_pd ();
        size_t i = 0;

        for (; i + 4 <= values.size(); i += 4) {

            __m256d values_vec = _mm256_loadu_pd (& values[i]);

            sum_vec = _mm256_add_pd(sum_vec , values_vec);

        }

        // Horizontal sum

        __m128d sum_high = _mm256_extractf128_pd(sum_vec , 1);

        __m128d sum_low = _mm256_castpd256_pd128(sum_vec);

        __m128d sum_final = _mm_add_pd(sum_high , sum_low);

        sum_final = _mm_hadd_pd(sum_final , sum_final);

        double result = _mm_cvtsd_f64(sum_final);

        // Add remaining elements

        for (; i < values.size(); ++i) {

            result += values[i];

        }

        return result;

    }

};



#endif // !VECTORIZED_OPARATIONS_H
