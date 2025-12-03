for (( i = 1; i <= 50; i = i+1)); do
  for (( j = 1; j <= 50; j = j+1)); do
    echo $i","$j","1
  done
done

for (( i = 51; i <= 100; i = i+1)); do
  for (( j = 51; j <= 100; j = j+1)); do
    echo $i","$j","1
  done
done

for (( i = 1; i <= 50; i = i+1)); do
  for (( j = 51; j <= 100; j = j+1)); do
    echo $i","$j","-1
  done
done

for (( i = 51; i <= 100; i = i+1)); do
  for (( j = 1; j <= 50; j = j+1)); do
    echo $i","$j","-1
  done
done
