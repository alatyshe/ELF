#pragma once

#include <chrono>

// ELF
#include "elf/logging/IndexedLoggerFactory.h"

// checkers
#include "../../common/DispatcherCallback.h"
#include "../distri_base.h"


using ThreadedCtrlBase = elf::ThreadedCtrlBase;

// Слушает сервер в
class ThreadedWriterCtrl : public ThreadedCtrlBase {
 public:
  ThreadedWriterCtrl(
      Ctrl& ctrl,
      const ContextOptions& contextOptions,
      const CheckersGameOptions& options)
      : ThreadedCtrlBase(ctrl, 0),
        logger_(elf::logging::getIndexedLogger(
            std::string("\x1b[1;35;40m|++|\x1b[0m") + 
            "ThreadedWriterCtrl-",
            "")) {
    elf::shared::Options netOptions = getNetOptions(contextOptions, options);
    writer_.reset(new elf::shared::Writer(netOptions));
    auto currTimestamp = time(NULL);
    logger_->info(
        "Writer info: {}, send ctrl with timestamp {} ",
        writer_->info(),
        currTimestamp);
    writer_->Ctrl(std::to_string(currTimestamp));

    start<>();
  }

  std::string identity() const {

    return writer_->identity();
  }

 protected:
  std::unique_ptr<elf::shared::Writer> writer_;
  int64_t seq_ = 0;
  uint64_t ts_since_last_sent_ = elf_utils::sec_since_epoch_from_now();
  std::shared_ptr<spdlog::logger> logger_;

  // Максимальное время ожидания ответа от сервера
  // потом идет переподключение
  static constexpr uint64_t kMaxSecSinceLastSent = 900;



  void on_thread() {
    std::string smsg;

    uint64_t now = elf_utils::sec_since_epoch_from_now();

    // Will block..
    // Постоянно ожидаем получить msg от сервера
    if (!writer_->getReplyNoblock(&smsg)) {
      logger_->info(
          "WriterCtrl: {}No message{}, seq={}, since_last_sec={}",
          ORANGE_C,
          COLOR_END,
          seq_,
          now - ts_since_last_sent_);

      // 900s = 15min
      if (now - ts_since_last_sent_ < kMaxSecSinceLastSent) {
        logger_->info("Sleep for 10 sec .. ");
        std::this_thread::sleep_for(std::chrono::seconds(10));
      } else {
        logger_->warn(
            "{}No reply for too long{} ({}>{} sec), resending",
            RED_B,
            COLOR_END,
            now - ts_since_last_sent_,
            kMaxSecSinceLastSent);
        getContentAndSend(seq_, false);
      }
      return;
    }

    logger_->info(
        "In reply func: {}Message got{}. since_last_sec={}, seq={}, \n{}",
        GREEN_B,
        COLOR_END,
        now - ts_since_last_sent_,
        seq_,
        smsg);

    json j = json::parse(smsg);
    MsgRequestSeq msg = MsgRequestSeq::createFromJson(j);

    ctrl_.sendMail("dispatcher", msg.request);

    getContentAndSend(msg.seq, msg.request.vers.wait());
  }





  void getContentAndSend(int64_t msg_seq, bool iswait) {

    if (msg_seq != seq_) {
      logger_->info(
          "Warning! The sequence number [{}] in the msg is different from {}",
          msg_seq,
          seq_);
    }
    std::pair<int, std::string> content;
    ctrl_.call(content);

    if (iswait) {
      std::this_thread::sleep_for(std::chrono::seconds(30));
    } else {
      if (content.first == 0)
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    writer_->Insert(content.second);
    seq_ = msg_seq + 1;
    ts_since_last_sent_ = elf_utils::sec_since_epoch_from_now();
  }
};












































// CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS
// CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS
// CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS
// CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS
// CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS
// CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS CHECKERS

struct CheckersGuardedRecords {
 public:
  CheckersGuardedRecords(const std::string& identity)
      : records_(identity),
        logger_(elf::logging::getIndexedLogger(
            std::string("\x1b[1;35;40m|++|\x1b[0m") + 
            "CheckersGuardedRecords",
            "")) {
  }

  void feed(const CheckersStateExt& s) {

    std::lock_guard<std::mutex> lock(mutex_);
    records_.addRecord(s.dumpRecord());
  }

  void updateState(const ThreadState& ts) {
    // call from CheckersGameNotifier::OnStateUpdate
    // Разобрать!!!!!!!!!!!!!!!!!!!!!!!
    // Разобрать!!!!!!!!!!!!!!!!!!!!!!!
    // Разобрать!!!!!!!!!!!!!!!!!!!!!!!
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = elf_utils::sec_since_epoch_from_now();

    // CheckersRecords records_;
    records_.updateState(ts);


    // добавляем инфу по текущей игре стата храниться в 
    // ThreadState
    // thread_id   - index
    // seq         - Which game we have played
    // move_idx    - Which move we have proceeded
    // black       - version of nn
    // white       - version of nn


    last_states_.push_back(std::make_pair(now, ts));
    if (last_states_.size() > 100) {
      last_states_.pop_front();
    }

    if (now - last_state_vis_time_ > 60) {
      std::unordered_map<int, ThreadState> states;
      std::unordered_map<int, uint64_t> timestamps;

      // last_states_ - std::deque<std::pair<uint64_t, ThreadState>>

      for (const auto& s : last_states_) {
        // unique index of ThreadState = just key to this ThreadState
        timestamps[s.second.thread_id] = s.first;
        states[s.second.thread_id] = s.second;
      }

      logger_->info(
          "{}UpdateStates{} {}",
          BLUE_C,
          COLOR_END,
          visStates(states, &timestamps));

      last_state_vis_time_ = now;
    }
  }

  size_t size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.records.size();
  }

  std::string dumpAndClear() {
    // send data.
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info(
        "{}DumpAndClear(dump all states to JSON and clean){}, #records: {}, {}",
        ORANGE_B,
        COLOR_END,
        records_.records.size(),
        visStates(records_.states));

    std::string s = records_.dumpJsonString();
    records_.clear();
    return s;
  }

 private:
  std::mutex mutex_;
  CheckersRecords records_;
  std::deque<std::pair<uint64_t, ThreadState>> last_states_;
  uint64_t last_state_vis_time_ = 0;
  std::shared_ptr<spdlog::logger> logger_;

  static std::string visStates(
      const std::unordered_map<int, ThreadState>& states,
      const std::unordered_map<int, uint64_t>* timestamps = nullptr) {
    // добавляем инфу по текущей игре. Статистика храниться в 
    // ThreadState
    // thread_id   - index
    // seq         - Which game we have played
    // move_idx    - Which move we have proceeded
    // black       - version of nn
    // white       - version of nn


    std::stringstream ss;
    ss << "#states: " << states.size();
    ss << "[";

    auto now = elf_utils::sec_since_epoch_from_now();
    std::vector<int> ordered;
    for (const auto& p : states) {
      ordered.push_back(p.first);
    }
    std::sort(ordered.begin(), ordered.end());

    for (const auto& th_id : ordered) {
      auto it = states.find(th_id);
      assert(it != states.end());

      ss  << "id:" << th_id 
          << ";seq:" << it->second.seq 
          << ";#move:" << it->second.move_idx;

      if (timestamps != nullptr) {
        auto it = timestamps->find(th_id);
        if (it != timestamps->end()) {
          uint64_t td = now - it->second;
          ss << ";timestamp:" << td;
        }
      }
      ss << ", ";
    }
    ss << "]  ";

    ss << elf_utils::get_gap_list(ordered);
    return ss.str();
  }
};













// используется в Client а так же в 
// GameSelfPlay : public GameBase можно увидеть в методе act(вся логика игры)
// CheckersGameNotifierBase 
class CheckersGameNotifier : public CheckersGameNotifierBase {
 public:
  CheckersGameNotifier(
      Ctrl& ctrl,
      const std::string& identity,
      const CheckersGameOptions& options,
      elf::GameClient* client)
      : ctrl_(ctrl), 
        records_(identity), 
        options_(options), 
        client_(client) {
    using std::placeholders::_1;
    using std::placeholders::_2;

    ctrl.RegCallback<std::pair<int, std::string>>(
        std::bind(&CheckersGameNotifier::dump_records, this, _1, _2));
  }

  void OnGameEnd(const CheckersStateExt& s) override {
    // tell python / remote
    records_.feed(s);

    game_stats_.resetRankingIfNeeded(options_.num_reset_ranking);
    game_stats_.feedWinRate(s.state().getFinalValue());
    // game_stats_.feedSgf(s.dumpSgf(""));

    // Report winrate (so that Python side could know).
    elf::FuncsWithState funcs =
        client_->BindStateToFunctions({end_target_}, &s);
    client_->sendWait({end_target_}, &funcs);
  }

  void OnStateUpdate(const ThreadState& state) override {
    // Update current state.
    records_.updateState(state);
  }

  void OnMCTSResult(Coord c, const CheckersGameNotifierBase::MCTSResult& result)
      override {
    // Check the ranking of selected move.
    auto move_rank =
        result.getRank(c, elf::ai::tree_search::MCTSResultT<Coord>::PRIOR);
    game_stats_.feedMoveRanking(move_rank.first);
  }

  GameStats& getGameStats() {
    return game_stats_;
  }

 private:
  Ctrl&                     ctrl_;
  GameStats                 game_stats_;
  CheckersGuardedRecords    records_;
  const CheckersGameOptions options_;
  elf::GameClient*          client_ = nullptr;
  const std::string         end_target_ = "game_end";

  bool dump_records(const Addr&, std::pair<int, std::string>& data) {
    data.first = records_.size();
    data.second = records_.dumpAndClear();
    return true;
  }
};

































































class DistriClient {
 public:
  DistriClient(
      const ContextOptions& contextOptions,
      const CheckersGameOptions& options,
      elf::GameClient* client)
      : contextOptions_(contextOptions),
        options_(options),
        logger_(elf::logging::getIndexedLogger(
          std::string("\x1b[1;35;40m|++|\x1b[0m") + 
          "DistriClient-", 
          "")) {
    dispatcher_.reset(new ThreadedDispatcher(ctrl_, contextOptions.num_games));
    dispatcher_callback_.reset(
        new DispatcherCallback(dispatcher_.get(), client));

    if (options_.mode == "selfplay") {
      
      writer_ctrl_.reset(
          new ThreadedWriterCtrl(ctrl_, contextOptions, options));

      checkers_game_notifier_.reset(
          new CheckersGameNotifier(ctrl_, writer_ctrl_->identity(), options, client));

    } else if (options_.mode == "online") {
    } else {
      throw std::range_error("options.mode not recognized! " + options_.mode);
    }
    logger_->info(
      "{}DistriClient successfully created{}\n",
      GREEN_B, 
      COLOR_END);
  }

  ~DistriClient() {
    // game_notifier_.reset(nullptr);
    checkers_game_notifier_.reset(nullptr);
    dispatcher_.reset(nullptr);
    writer_ctrl_.reset(nullptr);
  }

  ThreadedDispatcher* getDispatcher() {
    return dispatcher_.get();
  }

  CheckersGameNotifier* getCheckersNotifier() {
    return checkers_game_notifier_.get();
  }

  const GameStats& getCheckersGameStats() const {
    assert(checkers_game_notifier_ != nullptr);
    return checkers_game_notifier_->getGameStats();
  }

  // Used in client side.
  void setRequest(
      int64_t black_ver,
      int64_t white_ver,
      // float thres,
      int numThreads = -1) {
    MsgRequest request;
    request.vers.black_ver = black_ver;
    request.vers.white_ver = white_ver;
    request.vers.mcts_opt = contextOptions_.mcts_options;
    request.client_ctrl.num_game_thread_used = numThreads;
    dispatcher_->sendToThread(request);
  }

 private:
  Ctrl ctrl_;

  /// ZMQClient
  std::unique_ptr<ThreadedDispatcher>   dispatcher_;
  std::unique_ptr<ThreadedWriterCtrl>   writer_ctrl_;

  std::unique_ptr<DispatcherCallback>   dispatcher_callback_;
  std::unique_ptr<CheckersGameNotifier> checkers_game_notifier_;

  const ContextOptions                  contextOptions_;
  const CheckersGameOptions             options_;

  std::shared_ptr<spdlog::logger>       logger_;
};






