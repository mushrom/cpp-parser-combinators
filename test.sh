#!/bin/bash
for line in \
	"he runs." \
	"he likes her." \
	"can he run?" \
	"can he run to the store?" \
	"can Dave run a mile to the store?" \
	"will Dave run to the store?" \
	"he dislikes the sun, but he does like the moon." \
	"he dislikes the sun, but likes the moon." \
	"will you say \"hello\" to Dave at the convention?" \
	"I said \"Hello\" to Dave at the convention." \
	"it must have been raining very hard." \
	"it must have been raining at the beach." \
	"it must have been raining very hard at the beach." \
	"wow, it must have been raining very hard at the beach." \
	;
do
	echo "$line" | ./main data/naive_nlp.par
done;
