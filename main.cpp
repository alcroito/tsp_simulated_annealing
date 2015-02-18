#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <stack>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <random>

typedef std::string ID;

typedef struct Vertex {
    ID id;
    double x;
    double y;
} Vertex;

bool operator<(const Vertex& l, const Vertex& r) {
    return l.id < r.id;
}

bool operator==(const Vertex& l, const Vertex& r) {
    return l.id == r.id;
}

bool operator!=(const Vertex& l, const Vertex& r) {
    return l.id != r.id;
}

typedef std::vector<Vertex> VertexList;
typedef std::vector<ID> VertexIDList;
typedef std::unordered_map<ID, Vertex> VertexMap;
typedef std::mt19937 MyRNG;

typedef struct Rand {
    uint32_t seed_val;
    MyRNG rng;
    std::uniform_int_distribution<uint32_t> uint_dist;
    std::uniform_real_distribution<double> real_dist;

    Rand(uint32_t max) : seed_val((uint32_t) time(0)), uint_dist(0, max), real_dist(0.0, 1.0) {
        rng.seed(seed_val);
    }

    uint32_t getUInt() {
        uint32_t temp(uint_dist(rng));
        return temp;
    }

    double getDouble() {
        double temp(real_dist(rng));
        return temp;
    }

} Rand;

VertexList readVertexList() {
    std::fstream f("data.txt", std::fstream::in);
    std::fstream::iostate state = f.rdstate();
    std::cout << "File state: " << state << "\n";
    VertexList vertex_list;
    std::string line;

    while (std::getline(f, line)) {
        std::stringstream ss(line);
        Vertex c;
        ss >> c.id >> c.x >> c.y;
        vertex_list.push_back(c);
    }

    f.close();
    return vertex_list;
}

VertexList getAdjacentVertices(VertexList, Vertex);

VertexIDList depthFirstSearch(VertexList vertex_list) {
    VertexIDList dfs_path;
    std::stack<Vertex> st;
    std::set<Vertex> visited_set;
    std::cout << "Count of vertexlist " << vertex_list.size() << "\n";
    Vertex first = vertex_list.front();
    std::cout << "Count of vertexlist after " << vertex_list.size() << "\n";
    st.push(first);

    while (!st.empty()) {
        Vertex v = st.top();
        st.pop();

        auto vertexVisited = visited_set.find(v);
        if (vertexVisited == visited_set.end()) {
            // Add the vertex id to the traversed path, and mark as visited.
            dfs_path.push_back(v.id);
            visited_set.insert(v);

            VertexList adjacentVertices = getAdjacentVertices(vertex_list, v);
            std::for_each(adjacentVertices.begin(), adjacentVertices.end(), [&](Vertex one) {
                st.push(one);
            });
        }
    }

    return dfs_path;
}

VertexList getAdjacentVertices(VertexList vertex_list, Vertex initial_vertex) {
    VertexList adjacent_vertex_list;

    std::copy_if(vertex_list.begin(), vertex_list.end(), adjacent_vertex_list.begin(), [&](Vertex one) {
        return initial_vertex != one;
    });

    return adjacent_vertex_list;
}

VertexMap mapFromList(VertexList vertex_list) {
    VertexMap map;
    std::for_each(vertex_list.begin(), vertex_list.end(), [&](Vertex one) {
        map.emplace(one.id, one);
    });

    return map;
}

double cost(VertexIDList id_list, VertexMap vertex_map) {
    double theCost = 0;
    size_t count = id_list.size();
    for (size_t i = 0; i < count; i++) {
        auto id1 = id_list[i % count];
        auto id2 = id_list[(i+1) % count];
        Vertex first = vertex_map[id1];
        Vertex second = vertex_map[id2];
        theCost += sqrt(pow(first.x - second.x, 2) + pow(first.y - second.y, 2));
    }
    return theCost;
}

VertexIDList randomNeighbor(VertexIDList id_list, Rand& r) {
    VertexIDList neighbor_path = id_list;
    size_t vertex_count = id_list.size();
    uint32_t firstPos = r.getUInt() % vertex_count;
    uint32_t secondPos = r.getUInt() % vertex_count;
    std::swap(neighbor_path[firstPos], neighbor_path[secondPos]);
    return neighbor_path;
}

VertexIDList randomPath(VertexMap map) {
    VertexIDList list;
    // Initial list.
    for (auto kv : map) {
        list.push_back(kv.first);
    }

    // Randomly shuffle.
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(list.begin(), list.end(), g);
    return list;
}

double acceptance_probability(double old_cost, double new_cost, double t) {
    if ((new_cost < old_cost)) {
        return 1.0f;
    }
    return exp((old_cost - new_cost) / t);
}

VertexIDList anneal(VertexIDList initial, VertexMap map, Rand& r) {
    VertexIDList solution = initial;
    double old_cost = cost(initial, map);
    double t = 1.0f;
    double t_min = 0.0001f;
    double alpha = 0.9f;
    while (t > t_min) {
        uint32_t i = 1;
        VertexIDList best_solution_for_temp = initial;
        double best_solution_for_temp_cost = old_cost;
        while (i <= 100) {
            VertexIDList new_solution = randomNeighbor(solution, r);
            double new_cost = cost(new_solution, map);
            double ap = acceptance_probability(old_cost, new_cost, t);
            double rand_double = r.getDouble();
            if (ap > rand_double) {
                solution = new_solution;
                old_cost = new_cost;
            }
            if (new_cost < old_cost) {
                best_solution_for_temp = new_solution;
                best_solution_for_temp_cost = new_cost;
            }
            i++;
        }

        if (best_solution_for_temp_cost < old_cost) {
            solution = best_solution_for_temp;
            old_cost = best_solution_for_temp_cost;
        }

        t = t*alpha;
    }

    return solution;
}

VertexIDList repeated_anneal(VertexIDList initial, VertexMap map, Rand& r, uint32_t times = 10) {
    VertexIDList solution = initial;
    double current_cost = MAXFLOAT;
    for (uint32_t i = 0; i < times; i++) {
        VertexIDList new_solution = anneal(initial, map, r);
        double new_cost = cost(new_solution, map);
        if (new_cost < current_cost) {
            solution = new_solution;
            current_cost = new_cost;
        }
    }
    return solution;
}

int main()
{
    std::cout << "Reading vertices from file.\n";
    VertexList vertex_list = readVertexList();
    VertexMap vertex_map = mapFromList(vertex_list);

    // Prepare random generator.
    Rand r(vertex_list.size() - 1);

    std::cout << "Computing a random path through the graph.\n";
    VertexIDList path = randomPath(vertex_map);

    double path_cost = cost(path, vertex_map);
    std::cout << "Initial Cost: " << path_cost << "\n";

    std::cout << "Computing TSP approximate solution using repeated simulated annealing." << "\n";
    VertexIDList solution = repeated_anneal(path, vertex_map, r, 100);
    path_cost = cost(solution, vertex_map);

    std::cout << "Final path: \n";
    std::for_each(path.begin(), path.end(), [](ID id) {
        std::cout << id << "\n";
    });

    std::cout << "Found Solution Cost: " << path_cost << "\n";
    std::cout << "Optimal Solution Cost: " << 27603 << "\n";
    std::cout << "Finished\n";

    return 0;
}
