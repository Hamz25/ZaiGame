#include "NeuralNetwork.h"
#include <cmath>
#include <fstream>
#include <sstream>

std::mt19937& NeuralNetwork::rng() {
    // One shared, properly-seeded generator instead of constructing a new
    // std::mt19937 (and reseeding from random_device) on every single call,
    // which was slow and, on some platforms, poorly randomized when called
    // in a tight loop.
    static std::mt19937 generator(std::random_device{}());
    return generator;
}

NeuralNetwork::NeuralNetwork(const std::vector<int>& topology)
    : m_topology(topology)
{
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (size_t layer = 0; layer + 1 < topology.size(); ++layer) {
        int inCount  = topology[layer];
        int outCount = topology[layer + 1];

        std::vector<std::vector<float>> layerWeights(inCount, std::vector<float>(outCount));
        for (auto& row : layerWeights)
            for (float& w : row)
                w = dist(rng());
        m_weights.push_back(std::move(layerWeights));

        std::vector<float> layerBiases(outCount);
        for (float& b : layerBiases)
            b = dist(rng());
        m_biases.push_back(std::move(layerBiases));
    }
}

float NeuralNetwork::activateHidden(float x) {
    return std::tanh(x);
}

float NeuralNetwork::activateOutput(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

std::vector<float> NeuralNetwork::feedForward(const std::vector<float>& input) const {
    std::vector<float> current = input;

    for (size_t layer = 0; layer < m_weights.size(); ++layer) {
        const auto& layerWeights = m_weights[layer];
        const auto& layerBiases  = m_biases[layer];
        size_t outCount = layerBiases.size();

        std::vector<float> next(outCount, 0.0f);
        for (size_t from = 0; from < layerWeights.size(); ++from)
            for (size_t to = 0; to < outCount; ++to)
                next[to] += current[from] * layerWeights[from][to];

        bool isOutputLayer = (layer == m_weights.size() - 1);
        for (size_t to = 0; to < outCount; ++to) {
            next[to] += layerBiases[to];
            next[to] = isOutputLayer ? activateOutput(next[to]) : activateHidden(next[to]);
        }

        current = std::move(next);
    }

    return current;
}

void NeuralNetwork::mutate(float rate, float strength) {
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    std::normal_distribution<float> nudge(0.0f, strength);

    for (auto& layer : m_weights)
        for (auto& row : layer)
            for (float& w : row)
                if (chance(rng()) < rate)
                    w += nudge(rng());

    for (auto& layer : m_biases)
        for (float& b : layer)
            if (chance(rng()) < rate)
                b += nudge(rng());
}

NeuralNetwork NeuralNetwork::clone() const {
    NeuralNetwork copy;
    copy.m_topology = m_topology;
    copy.m_weights  = m_weights;
    copy.m_biases   = m_biases;
    return copy;
}

bool NeuralNetwork::saveToFile(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open())
        return false;

    out << m_topology.size() << '\n';
    for (int t : m_topology)
        out << t << ' ';
    out << '\n';

    // Interleaved per layer (weights then biases for layer 0, then layer 1, ...)
    // to match the order loadFromFile() reads them back in.
    for (size_t layer = 0; layer < m_weights.size(); ++layer) {
        for (const auto& row : m_weights[layer])
            for (float w : row)
                out << w << ' ';
        out << '\n';

        for (float b : m_biases[layer])
            out << b << ' ';
        out << '\n';
    }
    return true;
}

bool NeuralNetwork::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open())
        return false;

    size_t layerCount = 0;
    in >> layerCount;
    if (layerCount < 2)
        return false;

    std::vector<int> topology(layerCount);
    for (int& t : topology)
        in >> t;

    std::vector<std::vector<std::vector<float>>> weights;
    std::vector<std::vector<float>> biases;

    for (size_t layer = 0; layer + 1 < layerCount; ++layer) {
        int inCount  = topology[layer];
        int outCount = topology[layer + 1];

        std::vector<std::vector<float>> layerWeights(inCount, std::vector<float>(outCount));
        for (auto& row : layerWeights)
            for (float& w : row)
                if (!(in >> w)) return false;
        weights.push_back(std::move(layerWeights));

        std::vector<float> layerBiases(outCount);
        for (float& b : layerBiases)
            if (!(in >> b)) return false;
        biases.push_back(std::move(layerBiases));
    }

    m_topology = std::move(topology);
    m_weights  = std::move(weights);
    m_biases   = std::move(biases);
    return true;
}
