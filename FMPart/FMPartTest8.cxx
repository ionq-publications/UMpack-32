// Created by Igor Markov, June 2026

#include "ABKCommon/infolines.h"
#include "ABKCommon/paramproc.h"
#include "Partitioning/partProb.h"
#include "fmPart.h"

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
  const unsigned targetOffset = deterministicPart(nodeIdx, solIdx, numPartitions);
  unsigned seenAllowed = 0;
  for (unsigned part = 0; part < numPartitions; ++part) {
    if (!allowedParts.isInPart(part)) continue;
    if (seenAllowed == targetOffset % allowedParts.numberPartitions(numPartitions)) {
      return part;
    }
    ++seenAllowed;
  }
  abkfatal(0, "No legal partition found for deterministic start");
  return 0;
}

void initializeDeterministicStarts(PartitioningProblem& problem) {
  PartitioningBuffer& buffer = problem.getSolnBuffers();
  const Partitioning& fixedConstr = problem.getFixedConstr();
  const unsigned numPartitions = problem.getNumPartitions();

  for (unsigned sol = buffer.beginUsedSoln(); sol != buffer.endUsedSoln();
       ++sol) {
    for (unsigned node = 0; node < buffer[sol].size(); ++node) {
      const unsigned allowedCount =
          fixedConstr[node].numberPartitions(numPartitions);
      abkfatal(allowedCount > 0, "Node has no legal partitions");
      buffer[sol][node].setToPart(deterministicAllowedPart(
          fixedConstr[node], node, sol, numPartitions));
    }
  }
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

vector<unsigned> findCutEdges(const HGraphFixed& hgraph, const Partitioning& part) {
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

vector<CandidateSolution> collectCandidateSolutions(const char* baseFileName,
                                                    int argc,
                                                    const char* argv[]) {
  std::ostringstream hiddenStdout;
  std::ostringstream hiddenStderr;
  ScopedStreamCapture captureOut(std::cout, hiddenStdout);
  ScopedStreamCapture captureErr(std::cerr, hiddenStderr);

  PartitioningProblem::Parameters params(argc, argv);
  PartitionerParams pParams(argc, argv);
  pParams.eval = PartEvalType::NetCut2wayWWeights;

  PartitioningProblem problem(baseFileName, params);
  problem.discardHGraphNames();
  problem.reserveBuffers(kNumSolutions);
  setAllEdgeWeights(const_cast<HGraphFixed&>(problem.getHGraph()), 1);
  initializeDeterministicStarts(problem);

  MaxMem maxMem;
  FMPartitioner fm(problem, &maxMem, pParams, true);
  (void)fm;

  vector<CandidateSolution> candidates;
  PartitioningBuffer& buffer = problem.getSolnBuffers();
  for (unsigned sol = buffer.beginUsedSoln(); sol != buffer.endUsedSoln();
       ++sol) {
    const Partitioning& part = buffer[sol];
    const vector<unsigned> cutEdges = findCutEdges(problem.getHGraph(), part);
    unsigned cutEdgeIdx = UINT_MAX;
    bool hasCutEdge = !cutEdges.empty();
    if (hasCutEdge) {
      cutEdgeIdx = cutEdges[(sol - buffer.beginUsedSoln()) % cutEdges.size()];
    }

    CandidateSolution candidate = {part, hasCutEdge, cutEdgeIdx};
    candidates.push_back(candidate);
  }
  return candidates;
}

RefinementOutcome rerunFromSolution(const char* baseFileName, int argc,
                                    const char* argv[],
                                    const CandidateSolution& candidate,
                                    unsigned selectedWeight,
                                    WeightExperiment experiment) {
  std::ostringstream hiddenStdout;
  std::ostringstream hiddenStderr;
  ScopedStreamCapture captureOut(std::cout, hiddenStdout);
  ScopedStreamCapture captureErr(std::cerr, hiddenStderr);

  PartitioningProblem::Parameters params(argc, argv);
  PartitionerParams pParams(argc, argv);
  pParams.eval = PartEvalType::NetCut2wayWWeights;

  PartitioningProblem problem(baseFileName, params);
  problem.discardHGraphNames();
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

  MaxMem maxMem;
  FMPartitioner fm(problem, &maxMem, pParams, true, true);
  fm.runMultiStart();

  RefinementOutcome outcome = {
      true,
      !isEdgeCut(problem.getHGraph(), problem.getBestSoln(),
                 candidate.cutEdgeIdx)};
  return outcome;
}

ExperimentSummary runExperiment(const char* baseFileName, int argc,
                                const char* argv[],
                                const vector<CandidateSolution>& candidates,
                                WeightExperiment experiment) {
  ExperimentSummary summary = {
      false, vector<vector<WeightResult> >(candidates.size())};

  for (unsigned sol = 0; sol < candidates.size(); ++sol) {
    if (!candidates[sol].hasCutEdge) {
      for (unsigned weightIdx = 0; weightIdx < kNumWeights; ++weightIdx) {
        summary.allResults[sol].push_back(WeightResult{
            kWeights[weightIdx],
            RefinementOutcome{false, true}});
      }
      continue;
    }

    for (unsigned weightIdx = 0; weightIdx < kNumWeights; ++weightIdx) {
      const RefinementOutcome outcome =
          rerunFromSolution(baseFileName, argc, argv, candidates[sol],
                            kWeights[weightIdx], experiment);
      summary.allResults[sol].push_back(
          WeightResult{kWeights[weightIdx], outcome});
      summary.ranAtLeastOneExperiment =
          summary.ranAtLeastOneExperiment || outcome.ran;
    }
  }

  return summary;
}

void printResultsTable(const char* title,
                       const HGraphFixed& hgraph,
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

}  // namespace

// Deterministic heavy-edge sensitivity test.
// It first collects kNumSolutions candidate FM solutions on an all-unit-weight
// graph. For each solution, it picks one cut edge and re-runs FM once per
// entry in kWeights in three weighting modes. The re-run starts from the saved
// solution and reports whether the highlighted edge remains in the cut.
int main(int argc, const char* argv[]) {
  BoolParam helpRequest1("h", argc, argv);
  BoolParam helpRequest2("help", argc, argv);
  NoParams noParams(argc, argv);

  StringParam baseFileName("f", argc, argv);

  if (helpRequest1.found() || helpRequest2.found() || noParams.found()) {
    cout << " -f <aux filename>\n"
         << "Uses " << kNumSolutions
         << " multistart solutions and checks edge weights "
         << joinWeightList() << '.'
         << endl;
    return 0;
  }

  abkfatal(baseFileName.found(), " -f baseFileName not found");

  PartitioningProblem::Parameters statsParams(argc, argv);
  PartitioningProblem statsProblem(baseFileName, statsParams);
  statsProblem.discardHGraphNames();

  const vector<CandidateSolution> candidates =
      collectCandidateSolutions(baseFileName, argc, argv);
  abkfatal(candidates.size() == kNumSolutions,
           "Unexpected number of candidate solutions");

  cout << "\n FMPart large-weight handling test" << endl;
  cout << "---\n This test checks how FMPart handles large edge weights." << endl;
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
                      modes[modeIdx].experiment);
    printResultsTable(modes[modeIdx].title, statsProblem.getHGraph(),
                      candidates, summary);
    ranAtLeastOneExperiment =
        ranAtLeastOneExperiment || summary.ranAtLeastOneExperiment;
  }

  if (!ranAtLeastOneExperiment) {
    cout << "No candidate solution cut any edge, so weighted refinement was"
         << " skipped." << endl;
    return 0;
  }

  cout << "When an edge becomes many times heavier than its adjacent edges,\n"
       << "a smaller-cut solution exists. However, for a high-degree edge, "
       << "FM may not find it.\n"
       << "For a moderately heavier weight, what matters is how many "
       << "adjacent edges\n"
       << "must become cut to allow this edge to be uncut.\n";

  return 0;
}
