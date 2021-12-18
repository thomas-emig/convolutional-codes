SNR = [0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20];

% uncoded simulation results - 800.000.000 bits per error rate
% symlen = 2
  ber_uncoded_2 = [0.312970 0.258322 0.208010 0.160160 0.105965 0.051277 0.022462 0.005859 0.000488 0.000000 0.000000];
% symlen = 3
  ber_uncoded_3 = [0.431549 0.411090 0.388148 0.357455 0.317519 0.270850 0.217744 0.165212 0.115089 0.070126 0.033251];

% viterbi simulation results, block length: 40
%   8.000.000 bits for SNR <= 6 dB
%  80.000.000 bits for 6 dB < SNR <= 12 dB
% 800.000.000 bits for 12 dB < SNR
% code P0 = (1, 0, 1), P1 = (0, 1, 1)
  ber_coded_a =  [0.47644475 0.46117938 0.42904150 0.35369950 0.20225426 0.05556759 0.00530388 0.00010365 0.00000005 0.00000000 0.00000000];
  ber_coded_ah = [0.48286537 0.47121500 0.45056550 0.41244200 0.31488383 0.12610239 0.02951754 0.00213793 0.00001582 0.00000000 0.00000000];
% code P0 = (1, 0, 1, 1), P1 = (1, 1, 1, 0)
  ber_coded_b =  [0.39552138 0.34015763 0.25545600 0.14600025 0.05113465 0.00940691 0.00101320 0.00008345 0.00000425 0.00000000 0.00000000];
  ber_coded_bh = [0.42934537 0.38230362 0.31844975 0.23660138 0.12897549 0.03403435 0.00713818 0.00078870 0.00004199 0.00000000 0.00000000];
% code P0 = (1, 0, 1, 0, 1) P1 = (1, 1, 1, 1, 0)
  ber_coded_c =  [0.41049950 0.36191775 0.27795788 0.15592725 0.04874249 0.00756145 0.00073461 0.00004632 0.00000161 0.00000000 0.00000000];
  ber_coded_ch = [0.43898712 0.39839400 0.33869613 0.25346887 0.13034781 0.02799247 0.00501094 0.00055018 0.00003130 0.00000000 0.00000000];
% code P0 = (1, 0, 1, 1, 0, 1) P1 = (1, 1, 1, 0, 1, 0)
  ber_coded_d =  [0.41772113 0.37365175 0.29133975 0.16141488 0.04413277 0.00464203 0.00023770 0.00000723 0.00000004 0.00000000 0.00000000];
  ber_coded_dh = [0.44350963 0.40654625 0.35033875 0.26651000 0.13575105 0.02548014 0.00343851 0.00024433 0.00000988 0.00000000 0.00000000];
% code P0 = (1, 0, 1), P1 = (1, 1, 0), P2 = (0, 0, 1)
  ber_coded_e =  [0.45929863 0.43793962 0.40194325 0.34490962 0.25997444 0.15418703 0.06189733 0.01535534 0.00258657 0.00030119 0.00001673];
  ber_coded_eh = [0.47619238 0.46178962 0.44029487 0.40136738 0.33539321 0.23283224 0.12229943 0.04345182 0.01110718 0.00229275 0.00028805];

% stack simulation results, block length: 40
%   8.000.000 bits for SNR <= 6 dB
%  80.000.000 bits for 6 dB < SNR <= 12 dB
% 800.000.000 bits for 12 dB < SNR
% code P0 = (1, 0, 1), P1 = (0, 1, 1)
  ber_coded_a_stack =  [0.49438813 0.49045688 0.48059337 0.44454150 0.29595364 0.06324746 0.00526747 0.00010142 0.00000010 0.00000000 0.00000000];
  ber_coded_ah_stack = [0.48626600 0.47655087 0.45861225 0.42309575 0.32407505 0.12728738 0.02942048 0.00215187 0.00001465 0.00000000 0.00000000];
% code P0 = (1, 0, 1, 1), P1 = (1, 1, 1, 0)
  ber_coded_b_stack =  [0.44039262 0.40291987 0.34042350 0.23974513 0.11288014 0.02232641 0.00118016 0.00008396 0.00000457 0.00000000 0.00000000];
  ber_coded_bh_stack = [0.43927375 0.39857338 0.34322188 0.26866113 0.15908356 0.04353521 0.00910699 0.00117737 0.00007111 0.00000000 0.00000000];
% code P0 = (1, 0, 1, 0, 1) P1 = (1, 1, 1, 1, 0)
  ber_coded_c_stack =  [0.45139725 0.42164337 0.36524525 0.26114050 0.11822421 0.02064529 0.00088888 0.00004739 0.00000167 0.00000000 0.00000000];
  ber_coded_ch_stack = [0.44740812 0.41363787 0.36382138 0.28877063 0.16795185 0.04077811 0.00738501 0.00091822 0.00005630 0.00000000 0.00000000];
% code P0 = (1, 0, 1, 1, 0, 1) P1 = (1, 1, 1, 0, 1, 0)
  ber_coded_d_stack =  [0.45503312 0.42895213 0.37924375 0.27994800 0.12996135 0.02092715 0.00039911 0.00000770 0.00000010 0.00000000 0.00000000];
  ber_coded_dh_stack = [0.45056225 0.42028138 0.37606150 0.30543125 0.18120232 0.03992416 0.00526864 0.00042003 0.00002069 0.00000000 0.00000000];
% code P0 = (1, 0, 1), P1 = (1, 1, 0), P2 = (0, 0, 1)
  ber_coded_e_stack =  [0.48066500 0.46626375 0.44182763 0.40221638 0.33802003 0.23271180 0.08875951 0.02020420 0.00651650 0.00215000 0.00055342];
  ber_coded_eh_stack = [0.47619937 0.46284463 0.44303075 0.40807312 0.34727820 0.25427947 0.16141843 0.09084407 0.04723510 0.02125160 0.00551331];
% code P0 = (10001010110010100000101101001111) P1 = (11100010001111001000011000100111)
% block length = 50
  ber_coded_f_stack =  [0.46444470 0.44919250 0.42566920 0.38116360 0.27196315 0.04913198 0.00002175 0.00000069 0.00000014 0.00000000 0.00000000];
  ber_coded_fh_stack = [0.45818460 0.43800840 0.40892690 0.35773980 0.23181474 0.04969808 0.00885874 0.00135417 0.00009650 0.00000000 0.00000000];

close all;
figure();
plot(SNR - 3, ber_uncoded_2, '--^k', 'DisplayName', 'uncoded symbol length = 2');
hold all;
plot(SNR - 4, ber_uncoded_3, '--^k', 'DisplayName', 'uncoded symbol length = 3');
plot(SNR, ber_coded_a, '-*m', 'DisplayName', '[(101) (011)] viterbi soft');
plot(SNR, ber_coded_ah, '-om', 'DisplayName', '[(101) (011)] viterbi hard');
plot(SNR, ber_coded_b, '-*c', 'DisplayName', '[(1011) (1110)] viterbi soft');
plot(SNR, ber_coded_bh, '-oc', 'DisplayName', '[(1011) (1110)] viterbi hard');
plot(SNR, ber_coded_c, '-*r', 'DisplayName', '[(10101) (11110)] viterbi soft');
plot(SNR, ber_coded_ch, '-or', 'DisplayName', '[(10101) (11110)] viterbi hard');
plot(SNR, ber_coded_d, '-*g', 'DisplayName', '[(101101) (111010)] viterbi soft');
plot(SNR, ber_coded_dh, '-og', 'DisplayName', '[(101101) (111010)] viterbi hard');
plot(SNR, ber_coded_e, '-*b', 'DisplayName', '[(101) (110) (001)] viterbi soft');
plot(SNR, ber_coded_eh, '-ob', 'DisplayName', '[(101) (110) (001)] viterbi hard');

grid on;
set(gca, 'YScale', 'log');
xlabel('Channel SNR');
ylabel('Coded Bit Error Rate');
legend('Location', 'southwest');

figure();
plot(SNR - 3, ber_uncoded_2, '--^k', 'DisplayName', 'uncoded symbol length = 2');
hold all;
plot(SNR - 4, ber_uncoded_3, '--^k', 'DisplayName', 'uncoded symbol length = 3');
plot(SNR, ber_coded_a_stack, '-*m', 'DisplayName', '[(101) (011)] stack soft');
plot(SNR, ber_coded_ah_stack, '-om', 'DisplayName', '[(101) (011)] stack hard');
plot(SNR, ber_coded_b_stack, '-*c', 'DisplayName', '[(1011) (1110)] stack soft');
plot(SNR, ber_coded_bh_stack, '-oc', 'DisplayName', '[(1011) (1110)] stack hard');
plot(SNR, ber_coded_c_stack, '-*r', 'DisplayName', '[(10101) (11110)] stack soft');
plot(SNR, ber_coded_ch_stack, '-or', 'DisplayName', '[(10101) (11110)] stack hard');
plot(SNR, ber_coded_d_stack, '-*g', 'DisplayName', '[(101101) (111010)] stack soft');
plot(SNR, ber_coded_dh_stack, '-og', 'DisplayName', '[(101101) (111010)] stack hard');
plot(SNR, ber_coded_e_stack, '-*b', 'DisplayName', '[(101) (110) (001)] stack soft');
plot(SNR, ber_coded_eh_stack, '-ob', 'DisplayName', '[(101) (110) (001)] stack hard');
plot(SNR, ber_coded_f_stack, '-*k', 'DisplayName', 'WSPR code stack soft');
plot(SNR, ber_coded_fh_stack, '-ok', 'DisplayName', 'WSPR code stack hard');

grid on;
set(gca, 'YScale', 'log');
xlabel('Channel SNR');
ylabel('Coded Bit Error Rate');
legend('Location', 'southwest');
