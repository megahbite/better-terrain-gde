#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/tile_set.hpp>
#include <godot_cpp/classes/tile_data.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include <godot_cpp/variant/typed_dictionary.hpp>

#include <map>
#include <memory>
#include <vector>


namespace godot
{
    class Dictionary;
}

class BetterTerrainPP : public godot::Object
{
  GDCLASS(BetterTerrainPP, Object);

public:
  struct Placement
  {
    int source_id;
    godot::Vector2i coord;
    int alternative;
    std::map<int, std::vector<int>> peering;
    double probability;
  };

  struct ChangeWorkData
  {
    BetterTerrainPP* bt_ptr;
    std::vector<godot::Vector2i> cells;
    std::vector<const Placement*> placements;
    std::map<godot::Vector2i, int> types;
  };

  struct Changeset
  {
    bool valid;
    std::vector<godot::Vector2i> cells;
    godot::WorkerThreadPool::GroupID group_id;
  };

private:
  static Placement empty_placement;

  godot::TileMapLayer* m_tilemap;
  godot::Ref<godot::TileSet> m_tileset;
  std::map<int, std::vector<Placement>> m_cache;
  std::vector<int> m_terrain_types;
  std::map<godot::WorkerThreadPool::GroupID, const std::shared_ptr<ChangeWorkData>> m_work;

  bool m_fixed_random_seed{false};
  mutable godot::Ref<godot::RandomNumberGenerator> m_rng;

protected:
  static void _bind_methods();

public:
  BetterTerrainPP();

  bool init(godot::TileMapLayer* tilemap, bool fixed_random_seed = false);

  int get_cell(godot::Vector2i coord) const;
  bool set_cell(godot::Vector2i coord, int type);
  bool set_cells(const godot::Array& coords, int type);

  void update_terrain_cells(const godot::Array& cells, bool and_surrounding_cells = true);
  void update_terrain_cell(godot::Vector2i cell, bool and_surrounding_cells = true);
  void update_terrain_area(godot::Rect2i area, bool and_surrounding_cells = true);
  godot::Dictionary create_terrain_changeset(const godot::TypedDictionary<godot::Vector2i, int>& paint, bool and_surrounding_cells = true);
  bool is_terrain_changeset_ready(const godot::Dictionary& changeset);
  void wait_for_terrain_changeset(const godot::Dictionary& changeset);
  void apply_terrain_changeset(const godot::Dictionary& changeset);

  const Placement* update_tile_deferred(const godot::Vector2i& coord, const std::map<godot::Vector2i, int> &types) const;

private:
  std::vector<godot::Vector2i> widen(const std::vector<godot::Vector2i>& coords) const;
  std::vector<godot::Vector2i> widen_with_exclusion(const std::vector<godot::Vector2i>& coords, const godot::Rect2i& exclusion) const;
  const std::vector<int>& get_terrain_peering_cells() const;
  void update_tile_immediate(const godot::Vector2i& coord, const std::map<godot::Vector2i, int>& types) const;
  const Placement* update_tile_tiles(const godot::Vector2i& coord, const std::map<godot::Vector2i, int>& types, bool apply_empty_probability) const;
  const Placement* update_tile_vertices(const godot::Vector2i& coord, const std::map<godot::Vector2i, int>& types) const;
  int probe(godot::Vector2i coord, int peering, int type, const std::map<godot::Vector2i, int>& types) const;
  const Placement* weighted_selection(const std::vector<const Placement*>& choices, const godot::Vector2i& coord, bool apply_empty_probability) const;
};

