#ifndef RW_CUBE_QUAD_TREE
#define RW_CUBE_QUAD_TREE

#include <cinttypes>
#include <cmath>
#include <vector>
#include <memory>
#include <iterator>
#include <functional>
#include <random>
#include <algorithm>

namespace rw_cube {

template<typename T> struct PseudoQuadTree {
    struct Node {
        static constexpr auto INVALID_NEXT{ 0U };

        float x{ 0.F };
        float z{ 0.F };

        std::uint32_t next : 28{ INVALID_NEXT };
        bool has_child_00 : 1{ false };
        bool has_child_01 : 1{ false };
        bool has_child_10 : 1{ false };
        bool has_child_11 : 1{ false };
    }; // 12 bytes

    struct Leaf {
        T value;
        float x{ 0.F };
        float z{ 0.F };
    };

    static constexpr std::uint8_t MAX_TREE_HEIGHT{ 14U };

    std::vector<Node> heap_;
    std::vector<Leaf> leaves_;
    std::uint8_t tree_height_;
    float area_width_;
    float area_height_;
    float area_world_x_pos_;
    float area_world_z_pos_;

    PseudoQuadTree(
        std::uint8_t tree_height, 
        float area_width, 
        float area_height, 
        float area_world_x_pos,
        float area_world_z_pos) :
        // number elements in a heap = sum of geometrical series(a = 1, r = 4, n = tree_height + 1[+children])
        heap_(static_cast<std::size_t>((1 * (1 - static_cast<std::int32_t>(4U << (2U*(tree_height + 1))))) / (1 - 4))),
        tree_height_(tree_height + 1),
        area_width_(area_width),
        area_height_(area_height),
        area_world_x_pos_(area_world_x_pos),
        area_world_z_pos_(area_world_z_pos) {

        // https://en.wikipedia.org/wiki/Z-order_curve
        std::uint8_t quad_key_size{ 1U }; // 1, 2, 4, 6, 8 ...
        std::uint64_t quad_key{ 0 };
        std::uint64_t current_level_children_count{ 1 };
        std::uint32_t i{ 0 };
        float current_level_x_step{ area_width / 2.F };
        float current_level_z_step{ area_height / 2.F };
        for (auto& node : heap_) {
            const auto[x, z] = decodeQuadKey(quad_key, quad_key_size);
            node.x = current_level_x_step + static_cast<float>(2 * x) * current_level_x_step + area_world_x_pos;
            node.z = current_level_z_step + static_cast<float>(2 * z) * current_level_z_step + area_world_z_pos;
            node.next = 1 + (i * 4);
            if (++quad_key == current_level_children_count) {
                quad_key = 0;
                current_level_children_count *= 4U;
                current_level_x_step /= 2.F;
                current_level_z_step /= 2.F;
                quad_key_size += 2;
            }
            ++i;
        }
        // invalidate 'next' of the last level which holds leaves
        const auto leaves_level_beginning{ static_cast<std::uint32_t>(i - current_level_children_count/4) };
        for (std::uint32_t leaves_level_i{leaves_level_beginning}; leaves_level_i < i; ++leaves_level_i) {
            heap_[leaves_level_i].next = Node::INVALID_NEXT;
        }
    }

    void addToRandomLeaves(T value, std::uint32_t count) {
        std::random_device dev;
        std::mt19937 rng(dev());

        const auto last_level_extent = 2U << (tree_height_ - 1U);

        std::vector<std::pair<std::uint32_t, std::uint32_t>> 
        possible_discrete_positions(last_level_extent*last_level_extent);
        std::uint32_t xz{0}; 
        for (auto& position : possible_discrete_positions) {
            position.first = xz % last_level_extent;
            position.second = xz / last_level_extent;
            ++xz;
        }
        std::shuffle(
            possible_discrete_positions.begin(), 
            possible_discrete_positions.end(),
            rng
        );

        const auto leaf_x_step = area_width_ / static_cast<float>(last_level_extent);
        const auto leaf_x_offset = leaf_x_step/2.F;

        const auto leaf_z_step = area_height_ / static_cast<float>(last_level_extent);
        const auto leaf_z_offset = leaf_z_step/2.F;

        std::uniform_real_distribution<float> fDistX(
            -leaf_x_offset + leaf_x_offset/10.F, leaf_x_offset - leaf_x_offset/10.F
        );
        std::uniform_real_distribution<float> fDistZ(
            -leaf_z_offset + leaf_z_offset/10.F, leaf_z_offset - leaf_z_offset/10.F
        );

        for (std::uint32_t i{0}; i<count;) {
            const auto[rnd_ui_x, rnd_ui_z] = possible_discrete_positions.back();
            possible_discrete_positions.pop_back();

            const auto rnd_x = leaf_x_offset + static_cast<float>(rnd_ui_x) * leaf_x_step + area_world_x_pos_;
            const auto rnd_z = leaf_z_offset + static_cast<float>(rnd_ui_z) * leaf_z_step + area_world_z_pos_;

            float tmp_area_width = {area_width_/2.F};
            float tmp_area_height = {area_height_/2.F};
            std::uint8_t level{0};
            for (std::uint32_t j{0}; j<heap_.size();) {
                auto& node = heap_[j];

                if (level == tree_height_) {
                    if (node.next == Node::INVALID_NEXT) {
                        node.next = static_cast<std::uint32_t>(leaves_.size()) + 1U;
                        leaves_.emplace_back(Leaf{
                            .value = value,
                            .x = rnd_x + fDistX(rng),
                            .z = rnd_z + fDistZ(rng)
                        });
                        ++i;
                    }
                    break;
                }
                
                const auto node_child00 = heap_[node.next + 0];
                if (rnd_x >= node_child00.x - (tmp_area_width/2.F) && rnd_x < node_child00.x + (tmp_area_width/2.F)) {
                    if (rnd_z >= node_child00.z - (tmp_area_height/2.F) && rnd_z < node_child00.z + (tmp_area_height/2.F)) {
                    // in child 00
                        node.has_child_00 = true;
                        j = node.next + 0;
                    } else {
                    // in child 10
                        node.has_child_10 = true;
                        j = node.next + 2;

                    }
                } else {
                    if (rnd_z >= node_child00.z - (tmp_area_height/2.F) && rnd_z < node_child00.z + (tmp_area_height/2.F)) {
                    // in child 01
                        node.has_child_01 = true;
                        j = node.next + 1;
                    } else {
                    // in child 11
                        node.has_child_11 = true;
                        j = node.next + 3;
                    }
                }

                ++level;
                tmp_area_width /= 2.F;
                tmp_area_height /= 2.F;
            }
        }
    }

    static auto decodeQuadKey(std::uint64_t quad_key, std::uint8_t quad_key_size) {
        std::uint32_t x{ 0 };
        std::uint32_t z{ 0 };
        for (std::uint8_t i{0}; i<quad_key_size; i+=2) {
            x |= static_cast<std::uint32_t>((quad_key & (1UL << i)) >> (i/2));
            z |= static_cast<std::uint32_t>((quad_key & (1U << (i + 1))) >> (i/2 + 1));
        }

        return std::make_tuple(x, z);
    }

    struct Iterator {
        struct ValueType {
            Node node;
            float area_width;
            float area_height;
            std::uint8_t level;
        };

        const PseudoQuadTree<T>& tree_;
        std::function<void(const Leaf&)> value_action_;
        std::function<bool(ValueType)> predicate_;

        Iterator(
            const PseudoQuadTree<T>& tree, 
            std::function<void(const Leaf&)> value_action,
            std::function<bool(ValueType)> predicate = nullptr) :
            tree_(tree), 
            value_action_(std::move(value_action)),
            predicate_(std::move(predicate)){
        }

        void depthFirstTraversal() {
            const auto root = ValueType{ 
                .node = tree_.heap_.front(), 
                .area_width = tree_.area_width_,
                .area_height = tree_.area_height_,
                .level = 0
            };
            if (predicate_ == nullptr) {
                depthFirstTraversal(root);
            } else {
                if (predicate_(root)) {
                    depthFirstTraversalWithPredicate(root);
                }
            }
        }

    private:
        void depthFirstTraversal(ValueType value) {
            if (value.level == tree_.tree_height_) {
                if (value.node.next != Node::INVALID_NEXT) {
                    value_action_(tree_.leaves_[value.node.next - 1]);
                }
                return;
            }
            if (value.node.has_child_00) {
                depthFirstTraversal(ValueType{ 
                    .node = tree_.heap_[value.node.next + 0],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                });
            }
            if (value.node.has_child_01) {
                depthFirstTraversal(ValueType{ 
                    .node = tree_.heap_[value.node.next + 1],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                });
            }
            if (value.node.has_child_10) {
                depthFirstTraversal(ValueType{ 
                    .node = tree_.heap_[value.node.next + 2],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                });
            }
            if (value.node.has_child_11) {
                depthFirstTraversal(ValueType{ 
                    .node = tree_.heap_[value.node.next + 3],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                });
            }
        }
        void depthFirstTraversalWithPredicate(ValueType value) {
            if (value.level == tree_.tree_height_) {
                if (value.node.next != Node::INVALID_NEXT && predicate_(value)) {
                    value_action_(tree_.leaves_[value.node.next - 1]);
                }
                return;
            }
            if (value.node.has_child_00) {
                const auto new_value = ValueType{ 
                    .node = tree_.heap_[value.node.next + 0],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                };
                if (predicate_(new_value)) {
                    depthFirstTraversalWithPredicate(new_value);
                }
            }
            if (value.node.has_child_01) {
                const auto new_value = ValueType{ 
                    .node = tree_.heap_[value.node.next + 1],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                };
                if (predicate_(new_value)) {
                    depthFirstTraversalWithPredicate(new_value);
                }
            }
            if (value.node.has_child_10) {
                const auto new_value = ValueType{ 
                    .node = tree_.heap_[value.node.next + 2],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                };
                if (predicate_(new_value)) {
                    depthFirstTraversalWithPredicate(new_value);
                }
            }
            if (value.node.has_child_11) {
                const auto new_value = ValueType{ 
                    .node = tree_.heap_[value.node.next + 3],
                    .area_width = value.area_width/2.F, 
                    .area_height = value.area_height/2.F,
                    .level = static_cast<std::uint8_t>(value.level + 1)
                };
                if (predicate_(new_value)) {
                    depthFirstTraversalWithPredicate(new_value);
                }
            }
        }

    };
};

}

#endif