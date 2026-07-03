#pragma once
#include <vector>
#include <random>
#include <string>

// A small fully-connected feed-forward network used as a bird's "brain".
//
// Fixes vs. the original version:
//   - Added biases (a network with only weights and no biases can't
//     learn an offset threshold, which matters a lot for a jump/no-jump
//     decision).
//   - Added a non-linear activation function. The original network had
//     none, so stacking multiple weight layers was mathematically
//     equivalent to a single linear layer - the "hidden" layer was
//     doing nothing useful.
//   - Added clone()/save()/load() so genomes can be copied for the next
//     generation and the best genome can be persisted to disk.
class NeuralNetwork {
public:
    NeuralNetwork() = default;
    explicit NeuralNetwork(const std::vector<int>& topology);

    // Runs the network forward. input.size() must equal topology.front().
    std::vector<float> feedForward(const std::vector<float>& input) const;

    // Randomly perturbs weights/biases in place.
    //   rate     - probability [0,1] that any given weight/bias mutates
    //   strength - std-dev of the gaussian noise added on mutation
    void mutate(float rate, float strength);

    // Deep copy - used to carry a genome into the next generation.
    NeuralNetwork clone() const;

    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path);

    const std::vector<int>& topology() const { return m_topology; }
    bool isValid() const { return !m_topology.empty(); }

private:
    std::vector<int> m_topology;

    // m_weights[layer][fromNeuron][toNeuron] - weight connecting
    // neuron `fromNeuron` in layer `layer` to neuron `toNeuron` in layer `layer+1`.
    std::vector<std::vector<std::vector<float>>> m_weights;

    // m_biases[layer][neuron] - bias for `neuron` in layer `layer+1`.
    std::vector<std::vector<float>> m_biases;

    static float activateHidden(float x); // tanh
    static float activateOutput(float x); // sigmoid, so output lands in (0,1)
    static std::mt19937& rng();
};
