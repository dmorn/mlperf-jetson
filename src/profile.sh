#!/bin/sh

timeline=timeline-3.prof

echo "# Collecting timeline in $timeline"
/usr/local/cuda/bin/nvprof --profile-from-start off --cpu-thread-tracing on --profile-api-trace none --export-profile $timeline -f bin/infer kws01/*

# Display summary with
# /usr/local/cuda/bin/nvprof --import-profile $timeline
