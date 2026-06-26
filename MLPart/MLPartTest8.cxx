// Created by Igor Markov, June 2026

#include "ABKCommon/infolines.h"
#include "ABKCommon/paramproc.h"
#include "Partitioning/partProb.h"
#include "mlPart.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

using std::cout;
using std::endl;
using std::string;
using std::setw;
using std::vector;

namespace {

const unsigned kNumSolutions = 5;
const unsigned kClusteringWeightOptions[] = {0, 6};
const unsigned kNumClusteringWeightOptions =
    sizeof(kClusteringWeightOptions) / sizeof(kClusteringWeightOptions[0]);
const unsigned kWeights[] = {1, 5, 10, 100, 1000, 10000};
const unsigned kNumWeights = sizeof(kWeights) / sizeof(kWeights[0]);

struct CandidateSolution {
  Partitioning part;
  bool hasCutEdge;
  unsigned cutEdgeIdx;
};

struct RefinementOutcome {
  bool ran;
  bool passed;
};

struct WeightResult {
  unsigned weight;
  RefinementOutcome outcome;
};

enum WeightExperiment {
  SelectedWOthersOne,
  Selected20WOthersW,
  SelectedWPlus20000OthersW
};

struct ExperimentMode {
  const char* title;
  WeightExperiment experiment;
};

struct ExperimentSummary {
  bool ranAtLeastOneExperiment;
  vector<vector<WeightResult> > allResults;
};

class ScopedStreamCapture {
 public:
  ScopedStreamCapture(std::ostream& stream, std::ostringstream& capture)
      : _stream(stream), _oldBuf(stream.rdbuf(capture.rdbuf())) {}

  ~ScopedStreamCapture() { _stream.rdbuf(_oldBuf); }

 private:
  std::ostream& _stream;
  std::streambuf* _oldBuf;
};

void setAllEdgeWeights(HGraphFixed& hgraph, unsigned weight) {
  for (unsigned e = 0; e < hgraph.getNumEdges(); ++e)
    hgraph.getEdgeByIdx(e).setWeight(weight);
}

unsigned deterministicPart(unsigned nodeIdx, unsigned solIdx,
                           unsigned numPartitions) {
  const unsigned mixed = (nodeIdx * 1103515245u) ^ (solIdx * 12345u) ^
                         ((nodeIdx + 17u) * (solIdx + 31u));
  return mixed % numPartitions;
}

unsigned deterministicAllowedPart(const PartitionIds& allowedParts,
                                  unsigned nodeIdx, unsigned solIdx,
                                  unsigned numPartitions) {
  const unsigned targetOffset =
      deterministicPart(nodeIdx, solIdx, numPartitions);
  const unsigned numAllowed = allowedParts.numberPartitions(numPartitions);
  unsigned seenAllowed = 0;

  for (unsigned part = 0; part < numPartitions; ++part) {
    if (!allowedParts.isInPart(part)) continue;
    if (seenAllowed == targetOffset % numAllowed) return part;
    ++seenAllowed;
  }

  abkfatal(0, "No legal partition found for deterministic start");
  return 0;
}

void initializeDeterministicStart(PartitioningProblem& problem,
                                  unsigned solIdx) {
  PartitioningBuffer& buffer = problem.getSolnBuffers();
  const Partitioning& fixedConstr = problem.getFixedConstr();
  const unsigned numPartitions = problem.getNumPartitions();
  Partitioning& start = buffer[buffer.beginUsedSoln()];

  for (unsigned node = 0; node < start.size(); ++node) {
    const unsigned allowedCount =
        fixedConstr[node].numberPartitions(numPartitions);
    abkfatal(allowedCount > 0, "Node has no legal partitions");
    start[node].setToPart(deterministicAllowedPart(
        fixedConstr[node], node, solIdx, numPartitions));
  }
  problem.setBestSolnNum(buffer.beginUsedSoln());
}

MLPartParams makeTestParams(unsigned weightOption) {
  MLPartParams params(0, 0);
  params.eval = PartEvalType::NetCut2wayWWeights;
  params.seedTopLvlSoln = true;
  params.Vcycling = MLPartParams::NoVcycles;
  params.clParams.weightOption = weightOption;
  params.verb = Verbosity("0_0_0");
  return params;
}

bool isEdgeCut(const HGraphFixed& hgraph, const Partitioning& part,
               unsigned edgeIdx) {
  const HGFEdge& edge = hgraph.getEdgeByIdx(edgeIdx);
  itHGFNodeLocal nodeIt = edge.nodesBegin();
  abkfatal(nodeIt != edge.nodesEnd(), "Encountered empty edge");

  const unsigned firstPart = (*nodeIt)->getIndex() < part.size()
                                 ? part[(*nodeIt)->getIndex()].lowestNumPart()
                                 : UINT_MAX;
  for (++nodeIt; nodeIt != edge.nodesEnd(); ++nodeIt) {
    if (part[(*nodeIt)->getIndex()].lowestNumPart() != firstPart) return true;
  }
  return false;
}

vector<unsigned> findCutEdges(const HGraphFixed& hgraph,
                              const Partitioning& part) {
  vector<unsigned> cutEdges;
  for (unsigned e = 0; e < hgraph.getNumEdges(); ++e)
    if (isEdgeCut(hgraph, part, e)) cutEdges.push_back(e);
  return cutEdges;
}

string joinWeightList() {
  std::ostringstream out;
  for (unsigned i = 0; i < kNumWeights; ++i) {
    if (i) out << '/';
    out << kWeights[i];
  }
  return out.str();
}

CandidateSolution collectCandidateSolution(const char* baseFileName, int argc,
                                           const char* argv[],
                                           unsigned solIdx,
                                           unsigned weightOption) {
  std::ostringstream hiddenStdout;
  std::ostringstream hiddenStderr;
  ScopedStreamCapture captureOut(std::cout, hiddenStdout);
  ScopedStreamCapture captureErr(std::cerr, hiddenStderr);

  PartitioningProblem::Parameters problemParams(argc, argv);
  PartitioningProblem problem(baseFileName, problemParams);
  problem.discardHGraphNames();
  MLPartParams mlParams = makeTestParams(weightOption);
  problem.propagateTerminals(mlParams.partFuzziness);
  problem.reserveBuffers(1);

  setAllEdgeWeights(const_cast<HGraphFixed&>(problem.getHGraph()), 1);
  initializeDeterministicStart(problem, solIdx);

  BBPartBitBoardContainer bbBitBoards;
  MaxMem maxMem;
  MLPart mlPartitioner(problem, mlParams, bbBitBoards, &maxMem);
  (void)mlPartitioner;

  const Partitioning& bestPart = problem.getBestSoln();
  const vector<unsigned> cutEdges = findCutEdges(problem.getHGraph(), bestPart);
  unsigned cutEdgeIdx = UINT_MAX;
  const bool hasCutEdge = !cutEdges.empty();
  if (hasCutEdge) cutEdgeIdx = cutEdges[solIdx % cutEdges.size()];

  CandidateSolution candidate = {bestPart, hasCutEdge, cutEdgeIdx};
  return candidate;
}

vector<CandidateSolution> collectCandidateSolutions(const char* baseFileName,
                                                    int argc,
                                                    const char* argv[],
                                                    unsigned weightOption) {
  vector<CandidateSolution> candidates;
  for (unsigned sol = 0; sol < kNumSolutions; ++sol) {
    candidates.push_back(
        collectCandidateSolution(baseFileName, argc, argv, sol, weightOption));
  }
  return candidates;
}

RefinementOutcome rerunFromSolution(const char* baseFileName, int argc,
                                    const char* argv[],
                                    const CandidateSolution& candidate,
                                    unsigned selectedWeight,
                                    WeightExperiment experiment,
                                    unsigned weightOption) {
  std::ostringstream hiddenStdout;
  std::ostringstream hiddenStderr;
  ScopedStreamCapture captureOut(std::cout, hiddenStdout);
  ScopedStreamCapture captureErr(std::cerr, hiddenStderr);

  PartitioningProblem::Parameters problemParams(argc, argv);
  PartitioningProblem problem(baseFileName, problemParams);
  problem.discardHGraphNames();
  MLPartParams mlParams = makeTestParams(weightOption);
  problem.propagateTerminals(mlParams.partFuzziness);
  problem.reserveBuffers(1);

  HGraphFixed& hgraph = const_cast<HGraphFixed&>(problem.getHGraph());
  switch (experiment) {
    case SelectedWOthersOne:
      setAllEdgeWeights(hgraph, 1);
      hgraph.getEdgeByIdx(candidate.cutEdgeIdx).setWeight(selectedWeight);
      break;
    case Selected20WOthersW:
      setAllEdgeWeights(hgraph, selectedWeight);
      hgraph.getEdgeByIdx(candidate.cutEdgeIdx).setWeight(selectedWeight * 20);
      break;
    case SelectedWPlus20000OthersW:
      setAllEdgeWeights(hgraph, selectedWeight);
      hgraph.getEdgeByIdx(candidate.cutEdgeIdx).setWeight(selectedWeight +
                                                          20000);
      break;
  }

  problem.getSolnBuffers()[0] = candidate.part;
  problem.setBestSolnNum(0);

  BBPartBitBoardContainer bbBitBoards;
  MaxMem maxMem;
  MLPart mlPartitioner(problem, mlParams, bbBitBoards, &maxMem);
  (void)mlPartitioner;

  RefinementOutcome outcome = {
      true,
      !isEdgeCut(problem.getHGraph(), problem.getBestSoln(),
                 candidate.cutEdgeIdx)};
  return outcome;
}

ExperimentSummary runExperiment(const char* baseFileName, int argc,
                                const char* argv[],
                                const vector<CandidateSolution>& candidates,
                                WeightExperiment experiment,
                                unsigned weightOption) {
  ExperimentSummary summary = {
      false, vector<vector<WeightResult> >(candidates.size())};

  for (unsigned sol = 0; sol < candidates.size(); ++sol) {
    if (!candidates[sol].hasCutEdge) {
      for (unsigned weightIdx = 0; weightIdx < kNumWeights; ++weightIdx) {
        summary.allResults[sol].push_back(WeightResult{
            kWeights[weightIdx], RefinementOutcome{false, true}});
      }
      continue;
    }

    for (unsigned weightIdx = 0; weightIdx < kNumWeights; ++weightIdx) {
      const RefinementOutcome outcome =
          rerunFromSolution(baseFileName, argc, argv, candidates[sol],
                            kWeights[weightIdx], experiment, weightOption);
      summary.allResults[sol].push_back(
          WeightResult{kWeights[weightIdx], outcome});
      summary.ranAtLeastOneExperiment =
          summary.ranAtLeastOneExperiment || outcome.ran;
    }
  }

  return summary;
}

void printResultsTable(const char* title, const HGraphFixed& hgraph,
                       const vector<CandidateSolution>& candidates,
                       const ExperimentSummary& summary) {
  cout << title << endl;

  cout << setw(8) << "Soln" << setw(8) << "Edge" << setw(8) << "Degree";
  for (unsigned weightIdx = 0; weightIdx < kNumWeights; ++weightIdx) {
    std::ostringstream header;
    header << "W=" << kWeights[weightIdx];
    cout << setw(10) << header.str();
  }
  cout << endl << endl;

  for (unsigned sol = 0; sol < candidates.size(); ++sol) {
    cout << setw(8) << sol;
    if (candidates[sol].hasCutEdge) {
      cout << setw(8) << candidates[sol].cutEdgeIdx;
      cout << setw(8)
           << hgraph.getEdgeByIdx(candidates[sol].cutEdgeIdx).getDegree();
    } else {
      cout << setw(8) << "-";
      cout << setw(8) << "-";
    }
    for (unsigned resultIdx = 0; resultIdx < summary.allResults[sol].size();
         ++resultIdx) {
      if (!summary.allResults[sol][resultIdx].outcome.ran)
        cout << setw(10) << "SKIP";
      else
        cout << setw(10)
             << (summary.allResults[sol][resultIdx].outcome.passed ? "UNCUT"
                                                                    : "CUT");
    }
    cout << endl;
  }
  cout << endl;
}

bool runWeightOptionExperiment(const char* baseFileName, int argc,
                               const char* argv[],
                               const HGraphFixed& statsHGraph,
                               unsigned weightOption) {
  const vector<CandidateSolution> candidates =
      collectCandidateSolutions(baseFileName, argc, argv, weightOption);
  abkfatal(candidates.size() == kNumSolutions,
           "Unexpected number of candidate solutions");

  cout << "\nClustering weightOption: " << weightOption << endl;

  const ExperimentMode modes[] = {
      {"Weighted edge refinement results (selected=W, others=1)",
       SelectedWOthersOne},
      {"Weighted edge refinement results (selected=20*W, others=W)",
       Selected20WOthersW},
      {"Weighted edge refinement results (selected=W+20000, others=W)",
       SelectedWPlus20000OthersW}};
  const unsigned numModes = sizeof(modes) / sizeof(modes[0]);

  bool ranAtLeastOneExperiment = false;
  for (unsigned modeIdx = 0; modeIdx < numModes; ++modeIdx) {
    const ExperimentSummary summary =
        runExperiment(baseFileName, argc, argv, candidates,
                      modes[modeIdx].experiment, weightOption);
    printResultsTable(modes[modeIdx].title, statsHGraph, candidates, summary);
    ranAtLeastOneExperiment =
        ranAtLeastOneExperiment || summary.ranAtLeastOneExperiment;
  }

  return ranAtLeastOneExperiment;
}

}  // namespace

// Deterministic heavy-edge sensitivity test.
// It first collects kNumSolutions candidate MLPart solutions on an
// all-unit-weight graph. For each solution, it picks one cut edge and re-runs
// MLPart once per entry in kWeights in three weighting modes. The re-run
// starts from the saved solution and reports whether the highlighted edge
// remains in the cut.
int main(int argc, const char* argv[]) {
  BoolParam helpRequest1("h", argc, argv);
  BoolParam helpRequest2("help", argc, argv);
  NoParams noParams(argc, argv);

  StringParam baseFileName("f", argc, argv);

  if (helpRequest1.found() || helpRequest2.found() || noParams.found()) {
    cout << " -f <aux filename>\n"
         << "Uses " << kNumSolutions
         << " seeded MLPart solutions and checks edge weights "
         << joinWeightList() << ".\n"
         << "Repeats the experiment for clustering weight options 0 and 6.\n"
         << endl;
    return 0;
  }

  abkfatal(baseFileName.found(), " -f baseFileName not found");

  PartitioningProblem::Parameters statsParams(argc, argv);
  PartitioningProblem statsProblem(baseFileName, statsParams);
  statsProblem.discardHGraphNames();

  cout << "\n MLPart large-weight handling test" << endl;
  cout << "---\n This test checks how MLPart handles large edge weights." << endl;
  cout << "For each saved solution, one currently cut edge is selected for"
       << " refinement experiments." << endl;
  cout << "Increasing that edge's weight makes keeping it cut less attractive,"
       << " so refinement may improve the solution by making the edge uncut."
       << endl;
  cout << "The test reports three weighting cases: selected edge W with all"
       << " other edges at 1, selected edge 20*W with all other edges at W,"
       << " and selected edge W+20000 with all other edges at W." << endl
       << endl;
  cout << "Hypergraph statistics" << endl;
  statsProblem.getHGraph().printEdgeSizeStats();
  cout << " Edge weights for candidate collection: all set to 1" << endl;
  cout << " During refinement, the selected edge is made heavier according to"
       << " the mode described above to discourage it from remaining in the cut."
       << endl;
  statsProblem.getHGraph().printNodeDegreeStats();
  cout << endl;

  bool ranAtLeastOneExperiment = false;
  for (unsigned optIdx = 0; optIdx < kNumClusteringWeightOptions; ++optIdx) {
    const bool ranThisOption = runWeightOptionExperiment(
        baseFileName, argc, argv, statsProblem.getHGraph(),
        kClusteringWeightOptions[optIdx]);
    ranAtLeastOneExperiment = ranAtLeastOneExperiment || ranThisOption;
  }

  if (!ranAtLeastOneExperiment) {
    cout << "No candidate solution cut any edge, so weighted refinement was"
         << " skipped." << endl;
    return 0;
  }

  cout << "When an edge becomes many times heavier than its adjacent edges,\n"
       << "a smaller-cut solution exists. However, MLPart may not find it\n"
       << "because clustering and uncoarsening restrict the refinement path.\n"
       << "For a moderately heavier weight, what matters is how many "
       << "adjacent edges\n"
       << "must become cut to allow this edge to be uncut.\n";

  return 0;
}
