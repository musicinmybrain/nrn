# Notes on nrn#2027

## Summary

This file includes notes on the important points and code blocks for understanding the changes introduced in Olli's PR.
This should also give a good understanding of th whole NEURON architecture

### Data structure view

#### Hines algorithm related

```

struct NrnThread {
    double _t;
    double _dt;
    NrnThreadMembList* tml;
    Memb_list** _ml_list;
    int ncell;            /* analogous to old rootnodecount */
    int end;              /* 1 + position of last in v_node array. Now v_node_count. */
    int id;               /* this is nrn_threads[id] */

    std::size_t _node_data_offset{}; // Offset in the global node data where this NrnThread's values start.

    [[nodiscard]] double* node_area_storage();
    [[nodiscard]] double* node_rhs_storage();
    [[nodiscard]] double* node_voltage_storage();
    [[nodiscard]] double& actual_area(std::size_t row) {
        return node_area_storage()[row];
    }
    [[nodiscard]] double& actual_rhs(std::size_t row) {
        return node_rhs_storage()[row];
    }
    [[nodiscard]] double& actual_v(std::size_t row) {
        return node_voltage_storage()[row];
    }

    double* _actual_d;
    double* _actual_a;
    double* _actual_b;
    int* _v_parent_index;
    Node** _v_node;
    Node** _v_parent;
    double* _sp13_rhs;           /* rhs matrix for sparse13 solver. updates to and from this vector
                                    need to be transfered to actual_rhs */
    char* _sp13mat;              /* handle to general sparse matrix */

    _nrn_Fast_Imem* _nrn_fast_imem;
};

```

```
struct Model {
    void set_unsorted_callback(container::Mechanism::storage& mech_data);

    /** @brief One structure for all Nodes.
     */
    container::Node::storage m_node_data;

    /** @brief Storage for mechanism-specific data.
     *
     *  Each element is allocated on the heap so that reallocating this vector
     *  does not invalidate pointers to container::Mechanism::storage.
     */
    std::vector<std::unique_ptr<container::Mechanism::storage>> m_mech_data{};
};
```


#### Mechanism related


#### Lower level data structures

Struct that holds the tokens for Node and Mechanisms that control whether those are sorted
```
struct model_sorted_token {
    container::Node::storage::sorted_token_type node_data_token{};
    std::vector<container::Mechanism::storage::sorted_token_type> mech_data_tokens{};

  private:
    std::reference_wrapper<cache::Model> m_cache;
};
```

Structs for holding the offset data and pointers used during simulation
```
using Datum = neuron::container::generic_data_handle;
namespace neuron::cache {
struct Mechanism {
    /**
     * @brief Raw pointers into pointer data for use during simulation.
     *
     * pdata_ptr_cache contains pointers to the start of the storage for each pdata variable that is
     * flattened into the pdata member of this struct, and nullptr elsewhere. Compared to using
     * pdata directly this avoids exposing details such as the container used.
     */
    std::vector<double* const*> pdata_ptr_cache{};
    std::vector<std::vector<double*>> pdata{};      // raw pointers for use during simulation
    std::vector<std::vector<Datum*>> pdata_hack{};  // temporary storage used when populating pdata;
                                                    // should go away when pdata are SoA
};
struct Thread {
    /**
     * @brief Offset into global Node storage for this thread.
     */
    std::size_t node_data_offset{};
    /**
     * @brief Offsets into global mechanism storage for this thread (one per mechanism)
     */
    std::vector<std::size_t> mechanism_offset{};
};
struct Model {
    std::vector<Thread> thread{};
    std::vector<Mechanism> mechanism{};
};
extern std::optional<Model> model;
}  // namespace neuron::cache
```

