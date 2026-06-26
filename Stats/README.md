# Stats

Stats provides routines for computing and outputting statistics and
plots, including cumulative frequency distributions and correlation
metrics.

## Key Files

- `stats.h`
- `trivStats.h`
- `freqDistr.h`
- `corr.h`
- `linReg.h`
- `main0.cxx`
- `main3.cxx`

## Build

Optimized builds use `./makeOpt-gnu`.

Use the package wrapper from this directory:

```sh
./makeOpt-gnu
```

For a debug build, use `./make-gnu`.

## Test

Run the package regression script:

```sh
./regression
```

For direct statistics runs, these executables are the most useful entry
points:

- `StatsTest0`: summary-statistics smoke test using `TrivialStats`.
- `StatsTest1`: cumulative-frequency-distribution test and plot export.
- `StatsTest2`: correlation and linear-regression test.
- `StatsTest3`: loaded-die distribution test.
- `StatsTest4`: key counter and frequency-count test.
- `StatsTest5`: synthetic correlation and rank-correlation generator.
- `StatsTest9`: multiple regression test.
- `StatsTest10`: basic `ExpectedMins` and `TrivialStats` checks.
- `StatsTest20`: correlation sample with controlled random seeds.
- `StatsTest21`: correlated tuple generator and summary-statistics test.

## Synopsis

Routines for computing and outputting statistics and plots.

Main facilities include:

- `TrivialStats`
- `TrivialStatsWithStdDev`
- `CumulativeFrequencyDistribution`
- `Correlation`
- `RankCorrelation` (`RankCorr`)
- `LinearRegression` (`LinRegression`)

All classes accept statistical data in the form of `vector<double>` or
`vector<unsigned>`.

The first two classes can be used to quickly compute, query, and pretty-print
min/max/avg and standard deviation.

Example:

```cpp
vector<double> vec(3);
iota(vec.begin(), vec.end(), 0);
cout << TrivialStatsWithStdDev(vec);
```

Will produce:

```text
Max: 3  Min: 0  Avg: 1.5  StdDev: 1.25
```

For more info, see `trivStats.h` and `main0.cxx`.

`CumulativeFrequencyDistribution`, typedefed to `FreqDistr`, computes
cumulative frequency distributions, pretty-prints them, saves plots, and
allows several types of queries.

Example:

```cpp
vector<double> vec(4);
iota(vec.begin(), vec.end(), 0);
FreqDistr distr(vec);
cout << distr;
distr.saveXYPlot("plot.xy");
```

Will produce:

```text
Distribution in 4 equal subranges :
            [0,0.75]: 1             [0.75,1.5]: 1             [1.5,2.25]: 1
            [2.25,3]: 1
Distribution in 4 equipotent subranges :
               [0,2]: 3                  [2,2]: 0                  [2,3]: 1
               [3,3]: 0
```

The number of subranges can be set in user code. File `plot.xy` can be plotted
with gnuplot.

`FreqDistr` can be used in critical applications as well as for
pretty-printing.

Classes `Correlation`, `RankCorrelation` (`RankCorr`), and `LinearRegression`
(`LinRegression`) can be constructed from two `vector<double>` or two
`vector<unsigned>`.

`Correlation` and `RankCorr` have implicit conversions to `double`.
`LinRegression` has methods `double getC() const` and `double getK() const`.

For example, `Stats/main3.cxx` has the following lines:

```cpp
cout << " Correlation(vec1,vec2) = " << Correlation(vec1,vec2) << endl
     << " Correlation(vec1,vec3) = " << Correlation(vec1,vec3) << endl
     << " Correlation(vec1,vec4) = " << Correlation(vec1,vec4) << endl
     << " Correlation(vec4,vec5) = " << Correlation(vec4,vec5) << endl;

cout << " Rank Correlation(vec1,vec2) = " << RankCorr(vec1,vec2) << endl;
```
