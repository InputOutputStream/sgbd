#ifndef PARALLELIZATION_H

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include "page.hpp"


class ParallelTableScan {

    std::vector <Page*> pages;
    std:: unique_ptr <Predicate > where_condition;
    size_t num_threads;

    public:
        ParallelTableScan(std::vector <Page*> pages ,
                    std:: unique_ptr <Predicate > condition)
                : pages(std::move(pages)), where_condition(std::move(condition)
                 ),

        num_threads(std:: thread :: hardware_concurrency ()) {}

        std::vector <Record > execute () {
            std::vector <Record > results;
            std:: mutex results_mutex;
            #pragma omp parallel for num_threads(num_threads)
            for (size_t i = 0; i < pages.size(); ++i) {
                std::vector <Record > local_results;
                // Scan page i
                for (const auto& record : pages[i]->get_records ()) {
                        if (! where_condition || where_condition -> evaluate(record)) {
                            local_results.push_back(record);
                    }

                }
                // Merge results (critical section)
                #pragma omp critical
                {
                    results.insert(results.end(),
                    local_results.begin (),
                    local_results.end());
                }

            }
            return results;
        }

    void set_thread_count(size_t count) { num_threads = count; }
};



#endif // !PARALLELIZATION_H
