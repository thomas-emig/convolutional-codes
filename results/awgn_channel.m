SNR = [0, 2, 4, 6, 8, 10, 12, 14];

% uncoded simulation results - 800.000.000 bits per error rate
% symlen = 2
  ber_uncoded_2 = [0.07863759 0.03750666 0.01249095 0.00239028 0.00018975 0.00000376 0.00000001 0.00000000];
% symlen = 3
  ber_uncoded_3 = [0.12804897 0.08007474 0.04128646 0.01604836 0.00406057 0.00052290 0.00002290 0.00000021];

% viterbi simulation results, block length: 40
%     800.000 bits for         SNR <=  4 dB
%   8.000.000 bits for  4 dB < SNR <=  6 dB
%  80.000.000 bits for  6 dB < SNR <= 10 dB
% 800.000.000 bits for 10 dB < SNR
% code P0 = (1, 0, 1), P1 = (0, 1, 1)
  ber_coded_a =  [0.35548875 0.20219750 0.05668375 0.00545587 0.00013756 0.00000029 0.00000000 0.00000000];
  ber_coded_ah = [0.40942000 0.30961000 0.14543500 0.03102900 0.00223931 0.00003441 0.00000011 0.00000000];
% code P0 = (1, 0, 1, 1), P1 = (1, 1, 1, 0)
  ber_coded_b =  [0.14730250 0.05154750 0.00923000 0.00103700 0.00009347 0.00000717 0.00000031 0.00000000];
  ber_coded_bh = [0.23225375 0.12338125 0.04132875 0.00733887 0.00081245 0.00006590 0.00000271 0.00000003];
% code P0 = (1, 0, 1, 0, 1) P1 = (1, 1, 1, 1, 0)
  ber_coded_c =  [0.15763000 0.04929250 0.00740250 0.00074250 0.00005119 0.00000315 0.00000010 0.00000000];
  ber_coded_ch = [0.24909000 0.12475375 0.03412625 0.00525963 0.00057540 0.00005028 0.00000204 0.00000002];
% code P0 = (1, 0, 1, 1, 0, 1) P1 = (1, 1, 1, 0, 1, 0)
  ber_coded_d =  [0.16392500 0.04393375 0.00444625 0.00024775 0.00000886 0.00000012 0.00000000 0.00000000];
  ber_coded_dh = [0.26318250 0.12922125 0.03216750 0.00354537 0.00025038 0.00001565 0.00000067 0.00000000];
% code P0 = (1, 0, 1), P1 = (1, 1, 0), P2 = (0, 0, 1)
  ber_coded_e =  [0.11558875 0.03868500 0.00832625 0.00120975 0.00011611 0.00000462 0.00000002 0.00000000];
  ber_coded_eh = [0.18751250 0.08752625 0.02753250 0.00675150 0.00115258 0.00011710 0.00000528 0.00000009];

% stack simulation results, block length: 40
%     800.000 bits for         SNR <=  4 dB
%   8.000.000 bits for  4 dB < SNR <=  6 dB
%  80.000.000 bits for  6 dB < SNR <= 10 dB
% 800.000.000 bits for 10 dB < SNR
% code P0 = (1, 0, 1), P1 = (0, 1, 1)
  ber_coded_a_stack =  [0.45526625 0.34524375 0.11143750 0.00678712 0.00013500 0.00000076 0.00000000 0.00000000];
  ber_coded_ah_stack = [0.43682625 0.35070875 0.16290500 0.03168625 0.00226600 0.00003310 0.00000010 0.00000000];
% code P0 = (1, 0, 1, 1), P1 = (1, 1, 1, 0)
  ber_coded_b_stack =  [0.24071625 0.11323000 0.02341375 0.00135637 0.00009309 0.00000821 0.00000030 0.00000000];
  ber_coded_bh_stack = [0.26581000 0.14971250 0.05170000 0.00944637 0.00120999 0.00011809 0.00000496 0.00000004];
% code P0 = (1, 0, 1, 0, 1) P1 = (1, 1, 1, 1, 0)
  ber_coded_c_stack =  [0.26119000 0.11965000 0.02365250 0.00133875 0.00005121 0.00000288 0.00000009 0.00000000];
  ber_coded_ch_stack = [0.28445625 0.16166375 0.05038125 0.00793913 0.00094650 0.00009591 0.00000400 0.00000003];
% code P0 = (1, 0, 1, 1, 0, 1) P1 = (1, 1, 1, 0, 1, 0)
  ber_coded_d_stack =  [0.28148750 0.13206000 0.02383500 0.00080550 0.00000896 0.00000029 0.00000000 0.00000000];
  ber_coded_dh_stack = [0.30608875 0.17743625 0.05300250 0.00560350 0.00038147 0.00002591 0.00000101 0.00000001];
% code P0 = (1, 0, 1), P1 = (1, 1, 0), P2 = (0, 0, 1)
  ber_coded_e_stack =  [0.20872125 0.09705375 0.02438875 0.00210750 0.00012127 0.00000456 0.00000002 0.00000000];
  ber_coded_eh_stack = [0.25451375 0.14113750 0.05067625 0.00982525 0.00122960 0.00012074 0.00000526 0.00000009];
% code P0 = (10001010110010100000101101001111) P1 = (11100010001111001000011000100111)
% block length = 50
  ber_coded_f_stack =  [0.38435500 0.28113125 0.07790375 0.00026688 0.00000000 0.00000000 0.00000000 0.00000000];
  ber_coded_fh_stack = [0.36178625 0.23672500 0.06302750 0.00585300 0.00051586 0.00004573 0.00000190 0.00000000];

% fano simulation results, block length: 40
%     800.000 bits for         SNR <=  4 dB
%   8.000.000 bits for  4 dB < SNR <=  6 dB
%  80.000.000 bits for  6 dB < SNR <= 10 dB
% 800.000.000 bits for 10 dB < SNR
% code P0 = (1, 0, 1), P1 = (0, 1, 1)
  ber_coded_a_fano =  [0.47393500 0.34875250 0.06525875 0.00545450 0.00014972 0.00000015 0.00000000 0.00000000];
  ber_coded_ah_fano = [0.44036750 0.32450250 0.15040375 0.03049525 0.00227199 0.00004078 0.00000012 0.00000000];
% code P0 = (1, 0, 1, 1), P1 = (1, 1, 1, 0)
  ber_coded_b_fano =  [0.44592375 0.30697875 0.02937875 0.00104412 0.00009522 0.00000721 0.00000032 0.00000001];
  ber_coded_bh_fano = [0.33808000 0.17198000 0.04842000 0.00902937 0.00119820 0.00011619 0.00000488 0.00000004];
% code P0 = (1, 0, 1, 0, 1) P1 = (1, 1, 1, 1, 0)
  ber_coded_c_fano =  [0.44408750 0.30890750 0.02876375 0.00077562 0.00005399 0.00000319 0.00000010 0.00000000];
  ber_coded_ch_fano = [0.34338875 0.17295375 0.04328625 0.00721775 0.00094619 0.00008957 0.00000362 0.00000002];
% code P0 = (1, 0, 1, 1, 0, 1) P1 = (1, 1, 1, 0, 1, 0)
  ber_coded_d_fano =  [0.45008625 0.31472750 0.02327250 0.00026500 0.00001010 0.00000010 0.00000000 0.00000000];
  ber_coded_dh_fano = [0.35524750 0.17778750 0.03924625 0.00462487 0.00035841 0.00002620 0.00000098 0.00000000];
% code P0 = (1, 0, 1), P1 = (1, 1, 0), P2 = (0, 0, 1)
  ber_coded_e_fano =  [0.45535625 0.32750750 0.03955750 0.00127888 0.00012397 0.00000471 0.00000004 0.00000000];
  ber_coded_eh_fano = [0.41090625 0.22241625 0.04662125 0.00689588 0.00114084 0.00011634 0.00000532 0.00000010];
% code P0 = (10001010110010100000101101001111) P1 = (11100010001111001000011000100111)
% block length = 50
  ber_coded_f_fano =  [0.43386750 0.36141375 0.02161500 0.00000038 0.00000000 0.00000004 0.00000000 0.00000000];
  ber_coded_fh_fano = [0.39533750 0.20026875 0.01307250 0.00013000 0.00000734 0.00000090 0.00000002 0.00000000];

close all;
figure();
plot(SNR, ber_uncoded_2, '-ok', 'DisplayName', 'uncoded symbol length = 2');
hold all;
plot(SNR, ber_uncoded_3, '-sk', 'DisplayName', 'uncoded symbol length = 3');
plot(SNR, ber_coded_a, '-*m', 'DisplayName', '[(101) (011)] viterbi soft');
plot(SNR, ber_coded_ah, '--*m', 'DisplayName', '[(101) (011)] viterbi hard');
plot(SNR, ber_coded_b, '-*c', 'DisplayName', '[(1011) (1110)] viterbi soft');
plot(SNR, ber_coded_bh, '--*c', 'DisplayName', '[(1011) (1110)] viterbi hard');
plot(SNR, ber_coded_c, '-*r', 'DisplayName', '[(10101) (11110)] viterbi soft');
plot(SNR, ber_coded_ch, '--*r', 'DisplayName', '[(10101) (11110)] viterbi hard');
plot(SNR, ber_coded_d, '-*g', 'DisplayName', '[(101101) (111010)] viterbi soft');
plot(SNR, ber_coded_dh, '--*g', 'DisplayName', '[(101101) (111010)] viterbi hard');
plot(SNR, ber_coded_e, '-*b', 'DisplayName', '[(101) (110) (001)] viterbi soft');
plot(SNR, ber_coded_eh, '--*b', 'DisplayName', '[(101) (110) (001)] viterbi hard');

grid on;
set(gca, 'YScale', 'log');
xlabel('Eb/N0');
ylabel('Coded Bit Error Rate');
legend('Location', 'southwest');

figure();
plot(SNR, ber_uncoded_2, '-ok', 'DisplayName', 'uncoded symbol length = 2');
hold all;
plot(SNR, ber_uncoded_3, '-sk', 'DisplayName', 'uncoded symbol length = 3');
plot(SNR, ber_coded_a_stack, '-*m', 'DisplayName', '[(101) (011)] stack soft');
plot(SNR, ber_coded_ah_stack, '--*m', 'DisplayName', '[(101) (011)] stack hard');
plot(SNR, ber_coded_b_stack, '-*c', 'DisplayName', '[(1011) (1110)] stack soft');
plot(SNR, ber_coded_bh_stack, '--*c', 'DisplayName', '[(1011) (1110)] stack hard');
plot(SNR, ber_coded_c_stack, '-*r', 'DisplayName', '[(10101) (11110)] stack soft');
plot(SNR, ber_coded_ch_stack, '--*r', 'DisplayName', '[(10101) (11110)] stack hard');
plot(SNR, ber_coded_d_stack, '-*g', 'DisplayName', '[(101101) (111010)] stack soft');
plot(SNR, ber_coded_dh_stack, '--*g', 'DisplayName', '[(101101) (111010)] stack hard');
plot(SNR, ber_coded_e_stack, '-*b', 'DisplayName', '[(101) (110) (001)] stack soft');
plot(SNR, ber_coded_eh_stack, '--*b', 'DisplayName', '[(101) (110) (001)] stack hard');
plot(SNR, ber_coded_f_stack, '-*k', 'DisplayName', 'WSPR code stack soft');
plot(SNR, ber_coded_fh_stack, '--*k', 'DisplayName', 'WSPR code stack hard');

grid on;
set(gca, 'YScale', 'log');
xlabel('Eb/N0');
ylabel('Coded Bit Error Rate');
legend('Location', 'southwest');

figure();
plot(SNR, ber_uncoded_2, '-ok', 'DisplayName', 'uncoded symbol length = 2');
hold all;
plot(SNR, ber_uncoded_3, '-sk', 'DisplayName', 'uncoded symbol length = 3');
plot(SNR, ber_coded_a_fano, '-*m', 'DisplayName', '[(101) (011)] fano soft');
plot(SNR, ber_coded_ah_fano, '--*m', 'DisplayName', '[(101) (011)] fano hard');
plot(SNR, ber_coded_b_fano, '-*c', 'DisplayName', '[(1011) (1110)] fano soft');
plot(SNR, ber_coded_bh_fano, '--*c', 'DisplayName', '[(1011) (1110)] fano hard');
plot(SNR, ber_coded_c_fano, '-*r', 'DisplayName', '[(10101) (11110)] fano soft');
plot(SNR, ber_coded_ch_fano, '--*r', 'DisplayName', '[(10101) (11110)] fano hard');
plot(SNR, ber_coded_d_fano, '-*g', 'DisplayName', '[(101101) (111010)] fano soft');
plot(SNR, ber_coded_dh_fano, '--*g', 'DisplayName', '[(101101) (111010)] fano hard');
plot(SNR, ber_coded_e_fano, '-*b', 'DisplayName', '[(101) (110) (001)] fano soft');
plot(SNR, ber_coded_eh_fano, '--*b', 'DisplayName', '[(101) (110) (001)] fano hard');
plot(SNR, ber_coded_f_fano, '-*k', 'DisplayName', 'WSPR code fano soft');
plot(SNR, ber_coded_fh_fano, '--*k', 'DisplayName', 'WSPR code fano hard');

grid on;
set(gca, 'YScale', 'log');
xlabel('Eb/N0');
ylabel('Coded Bit Error Rate');
legend('Location', 'southwest');
