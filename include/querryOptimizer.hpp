#ifndef QUERRY_OPTIMIZER_H
#define QUERRY_OPTIMIZER_H

#include <memory>
#include <vector>
#include <variant>
#include <unordered_map>

class QueryOptimizer {
    struct TableStatistics {
        size_t row_count;
        size_t page_count;
        std:: unordered_map <std::string , ColumnStatistics > column_stats;
        std::vector <std:: unique_ptr <Index >> available_indexes;
    };

    struct ColumnStatistics {

        size_t distinct_values;
        size_t null_count;
        std::variant <int , double , std::string > min_value;
        std::variant <int , double , std::string > max_value;
        std::vector <std::pair <std::variant <int , double , std::string >,
        double >> histogram;
    };

    std:: unordered_map <std::string , TableStatistics > table_statistics;
    
    public:
        std:: unique_ptr <ExecutionPlan > optimize(const ASTNode& query);    
        void update_statistics(const std:: string& table_name);
    
    private:    
        double estimate_cost(const ExecutionPlan& plan);
        
        std::vector <std:: unique_ptr <ExecutionPlan >> generate_plans(const
        ASTNode& query);
        
        std:: unique_ptr <ExecutionPlan > select_best_plan(
        
        std::vector <std:: unique_ptr <ExecutionPlan >>& plans);


    // Optimization rules

    void push_down_selections(ExecutionPlan& plan);

    void push_down_projections(ExecutionPlan& plan);

    void choose_join_order(ExecutionPlan& plan);

    void choose_access_method(ExecutionPlan& plan);

    void consider_materialization(ExecutionPlan& plan);

};


#endif