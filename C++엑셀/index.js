const fs = require('fs');
const xlsx = require('xlsx');

function readFile()
{
    const settings = fs.readFileSync('settings.txt').toString().split('\n');
    const categories = settings[0].split(',');
    const types = settings[1].split(',');

    const book = xlsx.utils.book_new();

    {
        const arr = [];
        const names = [];
        for (type of types)
        {
            for (cate of categories)
            {
                names.push(`${type}_${cate}`);
            }
        }
        arr.push(names);
    
        const data = fs.readFileSync('Profile_Normal.txt').toString().split("\n");
    
        for (let i = 3; i < data.length;)
        {
            var temp = 0;
            var values = [];
    
            for (let j = 0; j < types.length; ++j, ++i)
            {
                var str = data[i].split('|');
    
                for (let t = 1; t <= categories.length; ++t)
                {
                    values.push(str[t].trim());
                }
            }
    
            arr.push(values);
    
            i += 4;
        }
    
        const normal = xlsx.utils.aoa_to_sheet(arr);
    
        const width = [];
        
        for (let i = 0; i < categories.length * types.length; ++i)
        {
            width.push({wpx: 150});
        }
    
        normal["!cols"] = width;
    
        xlsx.utils.book_append_sheet(book, normal, "NORMAL");
    }
    
    {
        const arr = [];
        const names = [];
        for (type of types)
        {
            for (cate of categories)
            {
                names.push(`${type}_${cate}`);
            }
        }
        arr.push(names);
    
        const data = fs.readFileSync('Profile_Biased.txt').toString().split("\n");

        for (let i = 3; i < data.length;)
        {
            var temp = 0;
            var values = [];
    
            for (let j = 0; j < types.length; ++j, ++i)
            {
                var str = data[i].split('|');
    
                for (let t = 1; t <= categories.length; ++t)
                {
                    values.push(str[t].trim());
                }
            }
    
            arr.push(values);
    
            i += 4;
        }
    
        const normal = xlsx.utils.aoa_to_sheet(arr);
    
        const width = [];
        
        for (let i = 0; i < categories.length * types.length; ++i)
        {
            width.push({wpx: 150});
        }
    
        normal["!cols"] = width;
    
        xlsx.utils.book_append_sheet(book, normal, "BIAS");
    }

    xlsx.writeFile(book, "result.xlsx");
}

readFile();