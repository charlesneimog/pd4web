SCORE_PATCH_TEMPLATE = """
#N canvas 610 152 324 402 8;
#X obj 5 5 inlet;
#X obj 5 165 s ui_pd4webscore{};
#X obj 5 72 route score delay;
#X obj 46 147 s ui_pd4webscore-delay{};
#X obj 5 102 symbol;
#X connect 0 0 2 0;
#X connect 2 0 4 0;
#X connect 2 1 3 0;
#X connect 4 0 1 0;
"""
