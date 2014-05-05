#ifndef SERIALIZE_MACRO_H_
#define SERIALIZE_MACRO_H_


#define CEREAL(...)                  \
  template <class Archive>           \
  void serialize(Archive& archive) { \
    archive(__VA_ARGS__);            \
  }


#define SERIALIZE_0()
#define SERIALIZE_1(A) CEREAL(cereal::make_nvp(#A, A))
#define SERIALIZE_2(A, B) CEREAL(cereal::make_nvp(#A, A), cereal::make_nvp(#B, B))
#define SERIALIZE_3(A, B, C) CEREAL(cereal::make_nvp(#A, A), cereal::make_nvp(#B, B), cereal::make_nvp(#C, C))
#define SERIALIZE_4(A, B, C, D) \
  CEREAL(cereal::make_nvp(#A, A), cereal::make_nvp(#B, B), cereal::make_nvp(#C, C), cereal::make_nvp(#D, D))
#define SERIALIZE_5(A, B, C, D, E) \
  CEREAL(cereal::make_nvp(#A, A),  \
         cereal::make_nvp(#B, B),  \
         cereal::make_nvp(#C, C),  \
         cereal::make_nvp(#D, D),  \
         cereal::make_nvp(#E, E))
#define SERIALIZE_6(A, B, C, D, E, F) \
  CEREAL(cereal::make_nvp(#A, A),     \
         cereal::make_nvp(#B, B),     \
         cereal::make_nvp(#C, C),     \
         cereal::make_nvp(#D, D),     \
         cereal::make_nvp(#E, E),     \
         cereal::make_nvp(#F, F))
#define SERIALIZE_7(A, B, C, D, E, F, G) \
  CEREAL(cereal::make_nvp(#A, A),        \
         cereal::make_nvp(#B, B),        \
         cereal::make_nvp(#C, C),        \
         cereal::make_nvp(#D, D),        \
         cereal::make_nvp(#E, E),        \
         cereal::make_nvp(#F, F),        \
         cereal::make_nvp(#G, G))
#define SERIALIZE_8(A, B, C, D, E, F, G, H) \
  CEREAL(cereal::make_nvp(#A, A),           \
         cereal::make_nvp(#B, B),           \
         cereal::make_nvp(#C, C),           \
         cereal::make_nvp(#D, D),           \
         cereal::make_nvp(#E, E),           \
         cereal::make_nvp(#F, F),           \
         cereal::make_nvp(#G, G),           \
         cereal::make_nvp(#H, H))
#define SERIALIZE_9(A, B, C, D, E, F, G, H, I) \
  CEREAL(cereal::make_nvp(#A, A),              \
         cereal::make_nvp(#B, B),              \
         cereal::make_nvp(#C, C),              \
         cereal::make_nvp(#D, D),              \
         cereal::make_nvp(#E, E),              \
         cereal::make_nvp(#F, F),              \
         cereal::make_nvp(#G, G),              \
         cereal::make_nvp(#H, H),              \
         cereal::make_nvp(#I, I))
#define SERIALIZE_10(A, B, C, D, E, F, G, H, I, J) \
  CEREAL(cereal::make_nvp(#A, A),                  \
         cereal::make_nvp(#B, B),                  \
         cereal::make_nvp(#C, C),                  \
         cereal::make_nvp(#D, D),                  \
         cereal::make_nvp(#E, E),                  \
         cereal::make_nvp(#F, F),                  \
         cereal::make_nvp(#G, G),                  \
         cereal::make_nvp(#H, H),                  \
         cereal::make_nvp(#I, I),                  \
         cereal::make_nvp(#J, J))
#define SERIALIZE_11(A, B, C, D, E, F, G, H, I, J, K) \
  CEREAL(cereal::make_nvp(#A, A),                     \
         cereal::make_nvp(#B, B),                     \
         cereal::make_nvp(#C, C),                     \
         cereal::make_nvp(#D, D),                     \
         cereal::make_nvp(#E, E),                     \
         cereal::make_nvp(#F, F),                     \
         cereal::make_nvp(#G, G),                     \
         cereal::make_nvp(#H, H),                     \
         cereal::make_nvp(#I, I),                     \
         cereal::make_nvp(#J, J),                     \
         cereal::make_nvp(#K, K))
#define SERIALIZE_12(A, B, C, D, E, F, G, H, I, J, K, L) \
  CEREAL(cereal::make_nvp(#A, A),                        \
         cereal::make_nvp(#B, B),                        \
         cereal::make_nvp(#C, C),                        \
         cereal::make_nvp(#D, D),                        \
         cereal::make_nvp(#E, E),                        \
         cereal::make_nvp(#F, F),                        \
         cereal::make_nvp(#G, G),                        \
         cereal::make_nvp(#H, H),                        \
         cereal::make_nvp(#I, I),                        \
         cereal::make_nvp(#J, J),                        \
         cereal::make_nvp(#K, K),                        \
         cereal::make_nvp(#L, L))
#define SERIALIZE_13(A, B, C, D, E, F, G, H, I, J, K, L, M) \
  CEREAL(cereal::make_nvp(#A, A),                           \
         cereal::make_nvp(#B, B),                           \
         cereal::make_nvp(#C, C),                           \
         cereal::make_nvp(#D, D),                           \
         cereal::make_nvp(#E, E),                           \
         cereal::make_nvp(#F, F),                           \
         cereal::make_nvp(#G, G),                           \
         cereal::make_nvp(#H, H),                           \
         cereal::make_nvp(#I, I),                           \
         cereal::make_nvp(#J, J),                           \
         cereal::make_nvp(#K, K),                           \
         cereal::make_nvp(#L, L),                           \
         cereal::make_nvp(#M, M))
#define SERIALIZE_14(A, B, C, D, E, F, G, H, I, J, K, L, M, N) \
  CEREAL(cereal::make_nvp(#A, A),                              \
         cereal::make_nvp(#B, B),                              \
         cereal::make_nvp(#C, C),                              \
         cereal::make_nvp(#D, D),                              \
         cereal::make_nvp(#E, E),                              \
         cereal::make_nvp(#F, F),                              \
         cereal::make_nvp(#G, G),                              \
         cereal::make_nvp(#H, H),                              \
         cereal::make_nvp(#I, I),                              \
         cereal::make_nvp(#J, J),                              \
         cereal::make_nvp(#K, K),                              \
         cereal::make_nvp(#L, L),                              \
         cereal::make_nvp(#M, M),                              \
         cereal::make_nvp(#N, N))
#define SERIALIZE_15(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O) \
  CEREAL(cereal::make_nvp(#A, A),                                 \
         cereal::make_nvp(#B, B),                                 \
         cereal::make_nvp(#C, C),                                 \
         cereal::make_nvp(#D, D),                                 \
         cereal::make_nvp(#E, E),                                 \
         cereal::make_nvp(#F, F),                                 \
         cereal::make_nvp(#G, G),                                 \
         cereal::make_nvp(#H, H),                                 \
         cereal::make_nvp(#I, I),                                 \
         cereal::make_nvp(#J, J),                                 \
         cereal::make_nvp(#K, K),                                 \
         cereal::make_nvp(#L, L),                                 \
         cereal::make_nvp(#M, M),                                 \
         cereal::make_nvp(#N, N),                                 \
         cereal::make_nvp(#O, O))
#define SERIALIZE_16(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P) \
  CEREAL(cereal::make_nvp(#A, A),                                    \
         cereal::make_nvp(#B, B),                                    \
         cereal::make_nvp(#C, C),                                    \
         cereal::make_nvp(#D, D),                                    \
         cereal::make_nvp(#E, E),                                    \
         cereal::make_nvp(#F, F),                                    \
         cereal::make_nvp(#G, G),                                    \
         cereal::make_nvp(#H, H),                                    \
         cereal::make_nvp(#I, I),                                    \
         cereal::make_nvp(#J, J),                                    \
         cereal::make_nvp(#K, K),                                    \
         cereal::make_nvp(#L, L),                                    \
         cereal::make_nvp(#M, M),                                    \
         cereal::make_nvp(#N, N),                                    \
         cereal::make_nvp(#O, O),                                    \
         cereal::make_nvp(#P, P))
#define SERIALIZE_17(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q) \
  CEREAL(cereal::make_nvp(#A, A),                                       \
         cereal::make_nvp(#B, B),                                       \
         cereal::make_nvp(#C, C),                                       \
         cereal::make_nvp(#D, D),                                       \
         cereal::make_nvp(#E, E),                                       \
         cereal::make_nvp(#F, F),                                       \
         cereal::make_nvp(#G, G),                                       \
         cereal::make_nvp(#H, H),                                       \
         cereal::make_nvp(#I, I),                                       \
         cereal::make_nvp(#J, J),                                       \
         cereal::make_nvp(#K, K),                                       \
         cereal::make_nvp(#L, L),                                       \
         cereal::make_nvp(#M, M),                                       \
         cereal::make_nvp(#N, N),                                       \
         cereal::make_nvp(#O, O),                                       \
         cereal::make_nvp(#P, P),                                       \
         cereal::make_nvp(#Q, Q))
#define SERIALIZE_18(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R) \
  CEREAL(cereal::make_nvp(#A, A),                                          \
         cereal::make_nvp(#B, B),                                          \
         cereal::make_nvp(#C, C),                                          \
         cereal::make_nvp(#D, D),                                          \
         cereal::make_nvp(#E, E),                                          \
         cereal::make_nvp(#F, F),                                          \
         cereal::make_nvp(#G, G),                                          \
         cereal::make_nvp(#H, H),                                          \
         cereal::make_nvp(#I, I),                                          \
         cereal::make_nvp(#J, J),                                          \
         cereal::make_nvp(#K, K),                                          \
         cereal::make_nvp(#L, L),                                          \
         cereal::make_nvp(#M, M),                                          \
         cereal::make_nvp(#N, N),                                          \
         cereal::make_nvp(#O, O),                                          \
         cereal::make_nvp(#P, P),                                          \
         cereal::make_nvp(#Q, Q),                                          \
         cereal::make_nvp(#R, R))
#define SERIALIZE_19(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S) \
  CEREAL(cereal::make_nvp(#A, A),                                             \
         cereal::make_nvp(#B, B),                                             \
         cereal::make_nvp(#C, C),                                             \
         cereal::make_nvp(#D, D),                                             \
         cereal::make_nvp(#E, E),                                             \
         cereal::make_nvp(#F, F),                                             \
         cereal::make_nvp(#G, G),                                             \
         cereal::make_nvp(#H, H),                                             \
         cereal::make_nvp(#I, I),                                             \
         cereal::make_nvp(#J, J),                                             \
         cereal::make_nvp(#K, K),                                             \
         cereal::make_nvp(#L, L),                                             \
         cereal::make_nvp(#M, M),                                             \
         cereal::make_nvp(#N, N),                                             \
         cereal::make_nvp(#O, O),                                             \
         cereal::make_nvp(#P, P),                                             \
         cereal::make_nvp(#Q, Q),                                             \
         cereal::make_nvp(#R, R),                                             \
         cereal::make_nvp(#S, S))
#define SERIALIZE_20(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T) \
  CEREAL(cereal::make_nvp(#A, A),                                                \
         cereal::make_nvp(#B, B),                                                \
         cereal::make_nvp(#C, C),                                                \
         cereal::make_nvp(#D, D),                                                \
         cereal::make_nvp(#E, E),                                                \
         cereal::make_nvp(#F, F),                                                \
         cereal::make_nvp(#G, G),                                                \
         cereal::make_nvp(#H, H),                                                \
         cereal::make_nvp(#I, I),                                                \
         cereal::make_nvp(#J, J),                                                \
         cereal::make_nvp(#K, K),                                                \
         cereal::make_nvp(#L, L),                                                \
         cereal::make_nvp(#M, M),                                                \
         cereal::make_nvp(#N, N),                                                \
         cereal::make_nvp(#O, O),                                                \
         cereal::make_nvp(#P, P),                                                \
         cereal::make_nvp(#Q, Q),                                                \
         cereal::make_nvp(#R, R),                                                \
         cereal::make_nvp(#S, S),                                                \
         cereal::make_nvp(#T, T))

// The interim macro that simply strips the excess and ends up with the required macro
#define SERIALIZE_X(x, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, FUNC, ...) FUNC

// The macro that the programmer uses
#define SERIALIZE(...)                   \
  SERIALIZE_X(,                          \
              ##__VA_ARGS__,             \
              SERIALIZE_20(__VA_ARGS__), \
              SERIALIZE_19(__VA_ARGS__), \
              SERIALIZE_18(__VA_ARGS__), \
              SERIALIZE_17(__VA_ARGS__), \
              SERIALIZE_16(__VA_ARGS__), \
              SERIALIZE_15(__VA_ARGS__), \
              SERIALIZE_14(__VA_ARGS__), \
              SERIALIZE_13(__VA_ARGS__), \
              SERIALIZE_12(__VA_ARGS__), \
              SERIALIZE_11(__VA_ARGS__), \
              SERIALIZE_10(__VA_ARGS__), \
              SERIALIZE_9(__VA_ARGS__),  \
              SERIALIZE_8(__VA_ARGS__),  \
              SERIALIZE_7(__VA_ARGS__),  \
              SERIALIZE_6(__VA_ARGS__),  \
              SERIALIZE_5(__VA_ARGS__),  \
              SERIALIZE_4(__VA_ARGS__),  \
              SERIALIZE_3(__VA_ARGS__),  \
              SERIALIZE_2(__VA_ARGS__),  \
              SERIALIZE_1(__VA_ARGS__),  \
              SERIALIZE_0(__VA_ARGS__))

#endif  // SERIALIZE_MACRO_H_
