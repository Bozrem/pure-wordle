for i in {1..10}; do
    time ./build/WordleSolver || break
    echo "Run $i ok"
done
