#include <iostream>
#include <vector>
#include <queue>

using namespace std;

typedef vector<size_t> VotesVector;
typedef vector<long> MetricsVector;

/**
 * This class implements calculation of vote metrics for the following Quora challenge:
 *
 * At Quora, we have aggregate graphs that track the number of upvotes we get each day.
 *
 * As we looked at patterns across windows of certain sizes, we thought about ways to track trends such as non-decreasing and
 * non-increasing subranges as efficiently as possible.
 *
 * For this problem, you are given N days of upvote count data, and a fixed window size K. For each window of K days, from
 * left to right, find the number of non-decreasing subranges within the window minus the number of non-increasing subranges
 * within the window.
 *
 * A window of days is defined as contiguous range of days. Thus, there are exactly N - K + 1 windows where this metric
 * needs to be computed. A non-decreasing subrange is defined as a contiguous range of indices [a,b], a < b, where each
 * element is at least as large as the previous element. A non-increasing subrange is similarly defined, except each element
 * is at least as large as the next. There are up to K(K-1)/2 of these respective subranges within a window, so the
 * metric is bounded by [-K(K-1)/2,K(K-1)/2].
 *
 * @see https://www.quora.com/about/challenges#upvotes for full task description
 */
class VoteMetricsCalculator {
private:
    typedef struct _Range {
        /**
         * Constructor.
         * Initial ranage is supposed to have at least 2 numbers (otherwise it's empty and
         * was not going to be created).
         */
        _Range(size_t start) : start(start), len(2) {}

        /**
         * Increases length of range by one, just like we'd push new eligible number to the
         * end of the range.
         */
        inline void pushBack()
        {
            len++;
        }

        /**
         * Moves start point 1 point forward and decreases length of range by one, jus like we'd
         * pop first number out of the range.
         */
        inline void popFront()
        {
            start++; len--;
        }

        /**
         * Checks if range is empty. We cannot have range smaller than of 2 numbers
         */
        inline bool empty()
        {
            return len < 2;
        }

        /**
         * Gets number of subranges for the current range.
         * The formula for non-empty range is: numSubranges = len*(len-1)/2.
         * For an empty range the result number of subranges is zero.
         */
        inline size_t getNumSubranges()
        {
            return !empty() ? ((len * (len - 1)) >> 1) : 0;
        }

        size_t start;
        size_t len;
    } Range;

    typedef queue<Range> RangesQueue;

public:
    // Constructor
    VoteMetricsCalculator(VotesVector& votes, const size_t winSize)
        : votes(votes), winSize(winSize), metrics(MetricsVector(votes.size() - winSize + 1, 0))
    {
    }

public:
    /**
     * Calculates metrics.
     *
     * In general case for the real-life task we supposed to have init/reset methods to
     * clear all statics before calculating metrics and let instance of this class to be
     * reused for updated votes/window size. However, following task description, we won't
     * make this class instance reusable. Instead we're just making sure that running this
     * method twice will produce correct result, and thus only necessary class fields
     * will be reset. For example we don't need to reset metrics, because unless votes or
     * winSize changed the metrics will be the same after multiple runs of the calculation.
     *
     * Additionally, we don't handle invalid cases here (those that are out of constraints).
     * Per description constraints are:
     * 1 <= N <= 100000 days
     * 1 <= K <= N days
     */
    void calculateMetrics()
    {
        /**
         * For the case when there's only 1 vote or winSize == 1, metrics are all zeros,
         * i.e. they are already calculated in initialization
         */
        if (votes.size() < 2 || winSize < 2) {
            return;
        }

        // Clear ranges queues
        upVoteRangesQueue = {};
        downVoteRangesQueue = {};

        // Flags to carry current range type.
        // Note that range may be of two states at the same time:
        // 1. Up, which is non-decreasing
        // 2. Down, which is non-increasing
        bool dirUp = false;
        bool dirDown = false;

        // Flag to handle case while we still don't have necessary number of
        // votes for the window of specified size.
        bool fullWindow = false;

        size_t currentWindow = 0;
        long currentWindowMetrics = 0;

        for (int pos = 1; pos < (int)votes.size(); pos++) {
            if (fullWindow) {
                currentWindowMetrics += pullFrontVote(pos);
                currentWindowMetrics += pushNextVote(pos, dirUp, dirDown);

                metrics[currentWindow++] = currentWindowMetrics;
            } else {
                currentWindowMetrics += pushNextVote(pos, dirUp, dirDown);

                if (pos + 1 == winSize) {
                    fullWindow = true;

                    metrics[currentWindow++] = currentWindowMetrics;
                }
            }
        }
    }

    /**
     * Gets metrics vector.
     */
    const MetricsVector& getMetricsVector() const
    {
        return this->metrics;
    }

protected:
    /**
     * Processes next vote, pushing it to the vote range queues as needed.
     *
     * Returns delta vote metrics for the current window.
     */
    long pushNextVote(const int& pos, bool& dirUp, bool& dirDown)
    {
        static auto pushNextVoteForRange = [](const int& pos, bool& dir, const bool& newDir, RangesQueue& voteRangesQueue)
        {
            long deltaVoteMetrics = 0;

            if (dir != newDir || voteRangesQueue.empty()) {
                dir = newDir;

                if (dir) {
                    voteRangesQueue.push(Range(pos - 1));
                    deltaVoteMetrics = 1;
                }
            } else {
                if (dir) {
                    Range& lastRange = voteRangesQueue.back();

                    deltaVoteMetrics -= lastRange.getNumSubranges();
                    lastRange.pushBack();
                    deltaVoteMetrics += lastRange.getNumSubranges();
                }
            }

            return deltaVoteMetrics;
        };

        bool newDirUp = votes[pos] >= votes[pos - 1];
        bool newDirDown = votes[pos] <= votes[pos - 1];

        long deltaUpVoteMetrics = pushNextVoteForRange(pos, dirUp, newDirUp, this->upVoteRangesQueue);
        long deltaDownVoteMetrics = pushNextVoteForRange(pos, dirDown, newDirDown, this->downVoteRangesQueue);

        return (deltaUpVoteMetrics - deltaDownVoteMetrics);
    }

    /**
     * Pulls out front vote, by pulling out 1 vote from related range queues.
     *
     * Returns delta vote metrics for the current window.
     */
    long pullFrontVote(const int& pos)
    {
        static auto pullFrontVoteForRange = [this](const int& pos, RangesQueue& voteRangesQueue)
        {
            long deltaVoteMetrics = 0;

            if (!voteRangesQueue.empty()) {
                Range& firstRange = voteRangesQueue.front();

                // Check if current range should loose any number because of pulling of front vote
                if (pos - winSize + 1 > firstRange.start) {
                    deltaVoteMetrics -= firstRange.getNumSubranges();

                    firstRange.popFront();

                    if (firstRange.empty()) {
                        voteRangesQueue.pop();
                    } else {
                        deltaVoteMetrics += firstRange.getNumSubranges();
                    }
                }
            }

            return deltaVoteMetrics;
        };

        long deltaUpVoteMetrics = pullFrontVoteForRange(pos, this->upVoteRangesQueue);
        long deltaDownVoteMetrics = pullFrontVoteForRange(pos, this->downVoteRangesQueue);

        return (deltaUpVoteMetrics - deltaDownVoteMetrics);
    }

private:
    VotesVector votes;
    size_t winSize;
    MetricsVector metrics;
    RangesQueue upVoteRangesQueue;
    RangesQueue downVoteRangesQueue;
};

int main()
{
    size_t N, K;
    cin >> N >> K;

    VotesVector votes(N);

    for (auto& i : votes) {
        cin >> i;
    }

    VoteMetricsCalculator calc(votes, K);
    calc.calculateMetrics();
    MetricsVector metrics = calc.getMetricsVector();

    for (auto& i : metrics) {
        cout << i << '\n';
    }

    cout.flush();

    return 0;
}