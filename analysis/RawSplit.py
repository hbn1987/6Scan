if __name__ == "__main__":
    file_name = "./Raw/raw_seeds_ICMP6_6Tree_2022425"
    lines = list()
    budget_count = dict()
    count = 0
    with open(file_name) as f:
        lines = f.read().splitlines()
    f.close()

    for line in lines:
        count += 1
        if line.find("Budget consumption") != -1:
            print(line, count)
            index = line.find(':') + 2
            budget = line[index:][::-1].replace("000000", 'M', 1)[::-1]
            budget_count[budget] = count
    
    label_1500M = False
    for budget, count in budget_count.items():
        if budget in ["10M", "50M", "100M", "200M", "400M", "1500M"]:
            new_file_name = file_name + '_' + budget
            if budget != "1500M":
                with open(new_file_name, 'w') as f:
                    for line in lines[:count]:
                        f.write("%s\n" % line)
                f.close()
            else:
                with open(new_file_name, 'w') as f:
                    for line in lines:
                        f.write("%s\n" % line)
                f.close()
                label_1500M = True

    if not label_1500M:
        new_file_name = file_name + "_1500M"
        with open(new_file_name, 'w') as f:
            for line in lines:
                f.write("%s\n" % line)
        f.close()