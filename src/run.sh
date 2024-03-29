make clean
make
directory="../traces"
echo "Starting Loop"
for file in "$directory"/*; do
       if [ -f "$file" ]; then
        # Check if file ends with ".bz2" extension
        if [[ "$file" == *.bz2 ]]; then
            # Extract and pipe the contents of the bz2 file to the predictor
            echo "-----------------------------------------------------------------------"
            echo "$file"
            bunzip2 -kc "$file" | ./predictor --static
            echo ""
            bunzip2 -kc "$file" | ./predictor --gshare:10
            echo
            bunzip2 -kc "$file" | ./predictor --tournament:10:10:10
            echo "-----------------------------------------------------------------------"
            # bunzip2 -kc "$file" | ./predictor --static
        fi
    fi
done

make clean