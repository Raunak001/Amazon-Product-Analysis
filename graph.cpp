#include "graph.h"

void Graph::insertVertex(int key) {
    if (adj_list.find(key) == adj_list.end()) {
        adj_list[key] = new Vertex(key);
    }
}

void Graph::insertEdge(int k1, int k2, int w) {
    if (adj_list.find(k1) != adj_list.end() && adj_list.find(k2) != adj_list.end()) {
        Vertex* v = adj_list[k1];
        Vertex* v2 = adj_list[k2];
        if (adj_list[k2]->degree <= adj_list[k1]->degree) {
            v = adj_list[k2];
            v2 = adj_list[k1];
        }
        for (auto edge : v->edges) {
            if (edge->v1 == v2 || edge->v2 == v2) {
                return;
            }
        }
        Edge* edge = new Edge(adj_list[k1], adj_list[k2], w);
        edge_list.push_back(edge);
        adj_list[k1]->edges.push_front(edge);
        adj_list[k1]->degree++;
        adj_list[k2]->edges.push_front(edge);
        adj_list[k2]->degree++;
    }
}

std::list<Graph::Edge*> Graph::incidentEdges(int key) {
    return adj_list[key]->edges;
}

bool Graph::areAdjacent(int k1, int k2) {
    if (adj_list.find(k1) != adj_list.end() && adj_list.find(k2) != adj_list.end()) {
        Vertex* v1 = adj_list[k1];
        Vertex* v2 = adj_list[k2];
        Vertex* v = v2;
        if (v1->degree <= v2->degree) {
            v = v1;
        }
        for(auto edge : v->edges) {
            if ((edge->v1 == v1 && edge->v2 == v2) || (edge->v1 == v2 && edge->v2 == v1)) {
                return true;
            }
        }
    }
    return false;
}

std::vector<int> Graph::BFS(int root, int target) {
    if (adj_list.find(root) == adj_list.end() || adj_list.find(target) == adj_list.end() ) {
        throw std::runtime_error("Specified Root or Target ID is not in the dataset. Retry with a valid ID(s).");
    }
    std::queue<int> q;
    std::unordered_map<int, int> path;
    q.push(root);
    while (!q.empty()) {
        int front = q.front();
        q.pop();
        if (front == target) {
            break;
        }
        for (auto edge : adj_list[front]->edges) {
            int neighbor = edge->v1->ID == front ? edge->v2->ID : edge->v1->ID;
            if (path.find(neighbor) == path.end()) {
                q.push(neighbor);
                path[neighbor] = front;
            } 
        }
    }
    std::vector<int> result;
    while (target != root) {
        result.push_back(target);
        target = path[target];
    }
    result.push_back(root);
    std::reverse(result.begin(), result.end());
    return result;
}

void Graph::Brandes_BFS_helper(int s, std::unordered_map<int,std::vector<int>>& pred, 
                                        std::unordered_map<int, int>& sigma, std::stack<int>& stack){
    std::unordered_map<int,int> dist;
    std::queue<int> q; 
    for (auto n : adj_list) {
        dist[n.first] = -1;
    }
    q.push(s);
    dist[s] = 0;
    while(!q.empty()) {
        int v = q.front();
        q.pop();
        stack.push(v);
        for (auto edge : adj_list[v]->edges) {
            int w = edge->v1->ID == v ? edge->v2->ID : edge->v1->ID;
            if (dist[w] == -1) {
                dist[w] = dist[v] + 1;
                q.push(w);
                
            }
            if (dist[w] == dist[v] + 1) {
                pred[w].push_back(v);
                sigma[w] = sigma[w] + sigma[v];

            }
        }
    }
}

std::vector<std::pair<int, float>> Graph::betweenness_centrality(int k) {
    if (k > (int) adj_list.size()) {
        throw std::runtime_error("Invalid. Betweenness centrality count greater than graph size. Retry with smaller BC #.");
    }
    std::map<int, float> CB;
    std::unordered_map<int,std::vector<int>> pred;
    std::unordered_map<int, int> sigma;
    std::unordered_map<int, int> delta;
    for (auto m : adj_list) {
        CB[m.first] = 0;
    }
    for (auto n : adj_list) {
        int s = n.first;
        pred.clear();
        std::stack<int> stack;
        for (auto m : adj_list) {
            delta[m.first] = 0;
            sigma[m.first] = 0;
        }
        sigma[s] = 1;
        Brandes_BFS_helper(s, pred, sigma, stack);
        while(!stack.empty()) {
            int w = stack.top();
            stack.pop();
            
            for (auto v : pred[w]) {
                delta[v] = delta[v] + 
                ((float) sigma[v] / (float) sigma[w]) * (1 + delta[w]);
            }
            if (w != s) {
                CB[w] = CB[w] + delta[w];
            }
        }
    }
    std::vector<std::pair<int, float>> results;
    for (auto& n : CB) {
        results.push_back(n);
        adj_list[n.first]->centrality = n.second;
        adj_list[n.first]->color = n.second;
    }
    std::sort(results.begin(), results.end(), [ ]( const std::pair<int, float>& first, const std::pair<int, float>& second ) {
        return first.second > second.second;
    });
    results.resize(k);
    bc_computed = true;
    return results;
}

Graph Graph::connected_subgraph(int root, int size) {
    if (adj_list.find(root) == adj_list.end()) {
        throw std::runtime_error("Specified Root ID is not in the dataset. Retry with a valid ID.");
    }
    std::queue<int> q;
    Graph g;
    std::unordered_map<int, int> visited;
    q.push(root);
    g.insertVertex(root);
    while (!q.empty()) {
        int front = q.front();
        q.pop();
        g.insertVertex(front);
        for (auto edge : adj_list[front]->edges) {
            int neighbor = edge->v1->ID == front ? edge->v2->ID : edge->v1->ID;
            if (visited.find(neighbor) == visited.end()) {
                q.push(neighbor);
                g.insertVertex(neighbor);
                g.insertEdge(front, neighbor, 1);
                if ((int) g.adj_list.size() == size) {
                    return g;
                }
                visited[neighbor]++;
                
            } 
        }
    }
    return g;
}

void Graph::calc_forces(float x, float y) {
    float C = 0.3;
    float k = C * std::sqrt((x*y)/((float) adj_list.size()));
    for (float t = 0; t < 1000; t++) {
        for (auto n : adj_list) {
            Vertex* v = n.second;
            for (auto m : adj_list) {
                Vertex* u = m.second;
                if (u != v && (u->pos.y != v->pos.y && u->pos.x != v->pos.x)) {
                    Vector diff = v->pos - u->pos;
                    v->disp = v->disp + (diff / diff.mag()) * ((k*k) / diff.mag());
                }
            }
        }
        for (auto e : edge_list) {
            Vector diff = e->v1->pos - e->v2->pos;
            e->v1->disp = e->v1->disp - (diff / diff.mag()) * ((diff.mag() * diff.mag()) / k);
            e->v2->disp = e->v2->disp - (diff / diff.mag()) * ((diff.mag() * diff.mag()) / k);
        }
        for (auto n : adj_list) {
            Vertex* v = n.second;
            v->pos = v->pos + v->disp / v->disp.mag();
        }
    }
}

void Graph::normalize_bc() {
    if (bc_computed) {
        int min = 1000000;
        int max = -1;
        for (auto n : adj_list) {
            if (n.second->centrality < min) {
                min = n.second->centrality;
            }
            if (n.second->centrality > max) {
                max = n.second->centrality;
            }
        }
        for (auto n : adj_list) {
            n.second->centrality = (n.second->centrality - min) * ( (float) (adj_list.size() / 5)/(max - min));
            n.second->color = (n.second->color - min) * ( (float) 255/(max - min));
            
        }
    }
}

void Graph::init_pos(int x, int y) {
    for (auto n : adj_list) {
        Vector pos_temp(std::rand() % x, std::rand() % y);
        n.second->pos = pos_temp;
    }
}

void Graph::set_dims(int& x, int& y) {
    int minx = 1000000, maxx = -1000000, miny = 1000000, maxy = -1000000;
    for (auto m : adj_list) {
        Vertex* v = m.second;
        if (v->pos.x < minx) {
            minx = v->pos.x;
        }
        if (v->pos.y < miny) {
            miny = v->pos.y;
        }
        if (v->pos.x > maxx) {
            maxx = v->pos.x;
        }
        if (v->pos.y > maxy) {
            maxy = v->pos.y;
        }
    }
    for (auto n : adj_list) {
        n.second->pos.x = n.second->pos.x - minx + 50;
        n.second->pos.y = n.second->pos.y - miny + 50;
    }
    x = maxx - minx + 100;
    y = maxy - miny + 100;
}

void Graph::draw_graph( std::string loc, int x1, int y1, bool ID) {
    if (loc != "") {
        loc = loc + "/";
    }
    std::ofstream ofs(loc+"GraphDrawn_Size" + std::to_string(adj_list.size()) +".svg"); 
    int m = 1;
    int x = x1 * m, y = y1 * m;
    init_pos(x, y);
    calc_forces(x, y);
    normalize_bc();
    set_dims(x, y);
    float scalex = (float) x / x1;
    float scaley = (float) y / y1;
    ofs << 
        "<!DOCTYPE svg>\n" <<
        "<svg version=\"1.1\"\n" <<
        "width=\""<<x1<<"\" height=\""<<y1<<"\"\n" << 
        "xmlns=\"http://www.w3.org/2000/svg\">\n\n";
    for (auto edge : edge_list) {
        int x1 = (edge->v1->pos.x) / scalex;
        int y1 = (edge->v1->pos.y) / scaley;
        int x2 = (edge->v2->pos.x) / scalex;
        int y2 = (edge->v2->pos.y) / scaley;
        ofs << "<line x1=\"" << x1 <<"\" y1=\""<< y1 <<"\" x2=\""<< x2 <<"\" y2=\""<< y2 <<"\" stroke-width=\"1\" stroke=\"skyblue\"/>\n";
    }
    for (auto v : adj_list) {
        Vertex* v1 = v.second;
        int x1 = (v1->pos.x) / scalex;
        int y1 = (v1->pos.y) / scaley;
        int r = v1->centrality == -1 ? 7 : v1->centrality;
        if (r <= 3) r = 3;
        ofs << "<circle cx=\""<< x1<<"\" cy=\""<<y1<<"\" r=\""<<r<<"\" fill=\""<< "rgb(" << 0<<", "<< v1->color<<", "<< 0 <<")\"/>\n";
        if (ID && r > 5)
            ofs << "<text x=\""<<x1<<"\" y=\""<<y1<<"\" text-anchor=\"middle\" stroke=\"red\" stroke-width=\"1px\" font-size=\""<<r<<"\" dy=\".3em\">" << v1->ID << "</text>\n";
    }
    ofs << "\n</svg>";
}

//O(V+E)
Graph::~Graph() {
    for (Edge* edge : edge_list) {
        delete edge;
    }
    for (auto v : adj_list) {
        delete v.second;
    }
}

